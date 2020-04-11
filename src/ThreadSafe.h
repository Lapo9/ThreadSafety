#ifndef THREAD_SAFE_OBJECT
#define THREAD_SAFE_OBJECT

#include <mutex>
#include <functional>
#include <vector>
#include <iostream> //DEBUG

namespace thread_safe {

	struct Locks;

	//a class that stores an object of any type and a mutex. Each time the object is accessed, the mutex is locked, and unlocked at the end of the access.
	template <typename WrappedType>
	class ThreadSafe {

		//the temporary class instantiated each time an object of type ThreadSafe is accessed and destroyed at the end of the access
		class Temp {
			ThreadSafe& real; //the reference to the permanent object
			std::lock_guard<std::mutex> guard {real.mtx};


			public:
			Temp(ThreadSafe& real) : real{real} {}


			//returns the object wrapped in ThreadSafe (pointer because -> needs a pointer as return type)
			WrappedType* operator->() {
				std::cout << "\x1B[31mTemp ->\033[0m\n"; //DEBUG
				return &real.wrappedObj;
			}

			//convert the temp type to the object it holds
			operator WrappedType&() {
				std::cout << "\x1B[31mTemp ->\033[0m\n"; //DEBUG
				return real.wrappedObj;
			}

		};

		WrappedType wrappedObj;
		std::mutex mtx;


		public:

		//constructs the wrapped object of type WrappedType using perfect forwarding
		template<typename ...ArgsType>
		ThreadSafe(ArgsType&&... args) : wrappedObj(std::forward<ArgsType>(args)...) {}
		
		ThreadSafe(ThreadSafe& ts) : wrappedObj{ts.wrappedObj} {}
		ThreadSafe& operator=(ThreadSafe& ts) {
			wrappedObj = ts.wrappedObj;
			return *this;
		}
		ThreadSafe(ThreadSafe&& ts) : wrappedObj{ std::move(ts.wrappedObj)} {}
		ThreadSafe& operator=(ThreadSafe&& ts) {
			wrappedObj = std::move(ts.wrappedObj);
		}
		


		//returns a temporary object which holds a reference to this object and locks the mutex on its creation. The temporary object has a -> which returns a reference to the wrapped object
		Temp operator->() {
			std::cout << "\x1B[31mThreadSafe ->\033[0m\n"; //DEBUG
			return Temp(*this);
		}

		//returns a temporary object which holds a reference to this object and locks the mutex on its creation. The temporary object has a cast operator which returns a reference to the wrapped object
		Temp operator*() {
			std::cout << "\x1B[31mThreadSafe *\033[0m\n"; //DEBUG
			return Temp(*this);
		}


		//TODO these 2 functions do the same thing, I have to choose one.
		//the first one is more "elegant", the operator has to be before the object, but it needs to be between () (because it has less precedence than .)
		//the second one needs no (), but it is more confusing

		//returns the wrapped object without locking the mutex
		WrappedType& operator~() {
			std::cout << "\x1B[31mThreadSafe ~\033[0m\n"; //DEBUG
			return wrappedObj;
		}

		WrappedType& operator--(int) {
			std::cout << "\x1B[31mThreadSafe --\033[0m\n"; //DEBUG
			return wrappedObj;
		}



		//the function to be called after the lockdown of the mutex
		template<typename Return, typename ...Args>
		void operator->*(std::function<Return(Args...)> callable) {
			std::lock_guard<std::mutex>(mtx);
			//TODO everything
		}		


		//if two ThreadSafe objects are separated by a comma, locks those objects' mutexes and return the object representing the list of the locked mutexes
		template <typename A, typename B>
		friend Locks operator,(ThreadSafe<A>& ts1, ThreadSafe<B>& ts2) {
			return Locks(std::unique_lock<std::mutex>(ts1.mtx), std::unique_lock<std::mutex>(ts2.mtx));
		}

		/*if a comma-separated list of locked mutexes and a ThreadSafe object is present, locks the new object's mutex and add it to the already existent list.
		Actually a new list is created, so that it is a temporary list, so it is destroyed as soon as the full expression containing the list ends.
		The order of the argument (Locks, ThreadSafe<A>) is relevant, since the comma operator as a left-to-right associativity. This way it is possible to write expressions of the kind `SomeType obj = (ts1, ts2, ts3, ..., tsN)->*foobar(...);`. (Note that the use of parentesis to enclose the comma separated list is necessary, sinche comma is the least prioritized operator in C++.*/
		//TODO maybe it is possible just to add the new locked mutex to the Locks list (instead of creating a new list)?
		template <typename A>
		friend Locks operator,(Locks locks, ThreadSafe<A>& ts) {
			return Locks(std::move(locks), std::unique_lock<std::mutex>(ts.mtx));
		}

	};


	//this object contains an array of unique_locks. It is instantiated temporarily, so that all of the ThreadSafe objects in a comma separated list are locked for the duration of the statement.
	//TODO there is the big problem that this mechanism is not deadlock safe. Example: 2 different threads run at the same time (ts1, ts2, ts3)->*... and (ts3, ts2, ts1)->*... The first thread locks ts1, the second thread locks ts3, the first locks ts2, the second tries to lock ts2 (but cannot), the first triest to lock ts3 (but cannot). A way to solve this is to make a thread release all of the mutex acquired in the comma separated list as soon as it fails to lock a mutex (and retry immediately), but there is the risk that the same situation goes on forever! Maybe it is possible to implement an algorithm similiar to the CSMACA (for wifi) but it would be slow (I think).
	struct Locks {
		//TODO maybe make the vector private?
		std::vector<std::unique_lock<std::mutex>> lockGuards;

		//constructs a Locks of 2 unique_lock(s)
		Locks(std::unique_lock<std::mutex>&& lg1, std::unique_lock<std::mutex>&& lg2) {
			lockGuards.push_back(std::move(lg1));
			lockGuards.push_back(std::move(lg2));
		}

		//constructs a Locks from another Locks and a unique_lock
		Locks(Locks&& locks, std::unique_lock<std::mutex>&& lg) {
			//TODO DEBUG uncomment next line and remove the for loop if you understand they are semantically equivalent
			//lockGuards = std::move(locks.lockGuards);
			for (auto& lock : locks.lockGuards) {
				lockGuards.push_back(std::move(lock));
			}

			lockGuards.push_back(std::move(lg));
		}

		//the function to be called after the lockdown of the mutexes
		template<typename Return, typename ...Args>
		void operator->*(std::function<Return(Args...)> callable) {
			//TODO everything
		}

		//DEBUG to remove
		void printTEST() { 
			std::cout << "\x1B[32mTEST\033[0m";
			std::cout << "\x1B[32m, lock count: \033[0m " << lockGuards.size() << "\n"; 
		}
	};
}

#endif
