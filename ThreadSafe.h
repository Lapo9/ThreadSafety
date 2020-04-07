#ifndef THREAD_SAFE_OBJECT
#define THREAD_SAFE_OBJECT

#include <mutex>

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
				return &real.wrappedObj;
			}
		};

		WrappedType wrappedObj;
		std::mutex mtx;


		public:

		//constructs the wrapped object of type WrappedType
		template<typename ...ArgsType>
		ThreadSafe(ArgsType... args) : wrappedObj(args...) {}

		//returns a temporary object which locks the mutex on its creation
		Temp operator->() {
			return Temp(*this);
		}

	};

}

#endif
