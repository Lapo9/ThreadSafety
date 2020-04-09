#ifndef THREAD_SAFE_OBJECT
#define THREAD_SAFE_OBJECT

#include <mutex>
#include <iostream> //DEBUG

namespace thread_safe {

	//a class that stores an object of any type and a mutex. Each time the object is accessed, the mutex is locked, and unlocked at the end of the access.
	template <typename WrappedType>
	class ThreadSafe {

		//the temporary class instantiated each time an object of type ThreadSafe is accessed and destroyed at the end of the access
		class Temp {
			ThreadSafe& real; //the reference to the permanent object
			std::lock_guard<std::mutex> guard {real.mtx};

			public:
			Temp(ThreadSafe& real) : real {real} {}

			//returns the object wrapped in ThreadSafe (pointer because -> needs a pointer as return type)
			WrappedType* operator->() {
				std::cout << "Temp ->\n"; //DEBUG
				return &real.wrappedObj;
			}

			//convert the temp type to the object it holds
			operator WrappedType&() {
				std::cout << "Temp impl cast\n"; //DEBUG
				return real.wrappedObj;
			}

		};

		WrappedType wrappedObj;
		std::mutex mtx;


		public:

		//constructs the wrapped object of type WrappedType using perfect forwarding
		template<typename ...ArgsType>
		ThreadSafe(ArgsType&&... args) : wrappedObj(std::forward<ArgsType>(args)...) {}

		//returns a temporary object which holds a reference to this object and locks the mutex on its creation. The temporary object has a -> which returns a reference to the wrapped object
		Temp operator->() {
			std::cout << "ThreadSafe ->\n"; //DEBUG
			return Temp(*this);
		}

		//returns a temporary object which holds a reference to this object and locks the mutex on its creation. The temporary object has a cast operator which returns a reference to the wrapped object
		Temp operator*() {
			std::cout << "ThreadSafe *\n"; //DEBUG
			return Temp(*this);
		}


		//TODO these 2 functions do the same thing, I have to choose one.
		//the first one is more "elegant", the operator has to be before the object, but it needs to be between () (because it has less precedence than .)
		//the second one needs no (), but it is more confusing

		//returns the wrapped object without locking the mutex
		WrappedType& operator~() {
			std::cout << "ThreadSafe ~\n"; //DEBUG
			return wrappedObj;
		}

		WrappedType& operator--(int) {
			std::cout << "ThreadSafe --\n"; //DEBUG
			return wrappedObj;
		}

	};

}

#endif
