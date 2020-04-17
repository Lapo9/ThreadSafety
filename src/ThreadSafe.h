#ifndef THREAD_SAFE_OBJECT
#define THREAD_SAFE_OBJECT

#include <mutex>
#include <functional>
#include <vector>
#include <iostream> //DEBUG

//C++20 templates needs to be restricted by concepts
namespace thread_safe {

	//TODO these are forward declarations. Maybe it is better to split the two classes in two different files
	class Locks;
	template <typename WrappedType>
	class ThreadSafe;

	//a shorter name for the class (not necessary)
	template <typename WrappedType>
	using ts = ThreadSafe<WrappedType>;

	//this object contains an array of unique_locks. It is instantiated temporarily, so that all of the ThreadSafe objects in a comma separated list are locked for the duration of the statement.
//TODO there is the big problem that this mechanism is not deadlock safe. Example: 2 different threads run at the same time (ts1, ts2, ts3)->*... and (ts3, ts2, ts1)->*... The first thread locks ts1, the second thread locks ts3, the first locks ts2, the second tries to lock ts2 (but cannot), the first triest to lock ts3 (but cannot). A way to solve this is to make a thread release all of the mutex acquired in the comma separated list as soon as it fails to lock a mutex (and retry immediately), but there is the risk that the same situation goes on forever! Maybe it is possible to implement an algorithm similiar to the CSMACA (for wifi) but it would be slow (I think).
	class Locks {
		std::vector<std::unique_lock<std::mutex>> lockGuards;

	public:
		//Constructs a Locks of 2 unique_lock(s)
		Locks(std::unique_lock<std::mutex>&& lg1, std::unique_lock<std::mutex>&& lg2) {
			lockGuards.push_back(std::move(lg1));
			lockGuards.push_back(std::move(lg2));
		}

		//add a new unique:lock to the array
		void add(std::unique_lock<std::mutex>&& lg) {
			lockGuards.push_back(std::move(lg));
		}

		//the function to be called after the lockdown of the mutexes
		template<typename Return>
		Return operator->*(std::function<Return()> callable) {
			return callable();
		}

		//DEBUG to remove
		int printTEST() {
			std::cout << "\x1B[32mTEST\033[0m";
			std::cout << "\x1B[32m, lock count: \033[0m " << lockGuards.size() << "\n";
			return 14;
		}
	};



	/**
	 * @class ThreadSafe
	 * @brief The class associates an object of type WrappedType with a mutex. Each time the object is accessed with `->` or `*` operators, the associated mutex is locked.
	 * @details The mutex is locked with a unique_lock encapsulated in a temporary object anonimously returned by the `->` and `*` operators. Such temporary object defines a `->` operator which returns a pointer to the object wrapped in the ThreadSafe object who created it. Due to arrow operation concatenation rule of C++, this the operator is called whenever the `->` is called on the ThreadSafe object.
	 * The class provides an overload of `~` operator to access the wrapped in object in a non thread-safe way without any overhead.
	 * The `,` operator is also overloaded in order to protect multiple ThreadSafe objects at the same time and perform any operation avoiding data races on the protected objects.
	 * Referencing an object in the same statement in which it has been locked will cause a deadlock. This means that in a statement where the object is referenced throught `->` or `*` or appears in a comma separated list of ThreadSafe objects, it should be only be accessed throught `~` operator.
	**/
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

		/**
		 * @brief Constructs a ThreadSafe object which stores an object of type WrappedType.
		 * @details The arguments provided are perfecly forwarded to the constructor of the WrappedType object.
		 * @tparam ArgsType The type of the arguments accepted by the constructor of the WrappedType object.
		 * @param args The arguments passed to the constructor of the WrappedType object via perfect forwarding.
		**/
		template<typename ...ArgsType>
		ThreadSafe(ArgsType&&... args) : wrappedObj(std::forward<ArgsType>(args)...) {}
		
		/**
		 * @brief Constructs a new ThreadSafe object using the copy constructor of the WrappedType object.
		 * @details The internal mutex is not copyed, but is newly initialized.
		 * @param ts ThreadSafe obeject to copy.
		**/
		ThreadSafe(ThreadSafe& ts) : wrappedObj{ts.wrappedObj} {}

		/**
		 * @brief Constructs a new ThreadSafe object using the copy '=' operator of the WrappedType object.
		 * @details The internal mutex is not copyed, but is newly initialized.
		 * @param ts ThreadSafe obeject to copy.
		**/
		ThreadSafe& operator=(ThreadSafe& ts) {
			wrappedObj = ts.wrappedObj;
			return *this;
		}

		//TODO what happens if an object is moved with the mutex locked? Can it even happen?
		/**
		 * @brief Moves the ThreadSafe object using the move constructor of the WrappedType object.
		 * @details The internal mutex is not moved, but is newly initialized.
		 * @param ts ThreadSafe obeject to move.
		**/
		ThreadSafe(ThreadSafe&& ts) : wrappedObj{ std::move(ts.wrappedObj)} {}

		/**
		 * @brief Moves the ThreadSafe object using the move '=' operator of the WrappedType object.
		 * @details The internal mutex is not moved, but is newly initialized.
		 * @param ts ThreadSafe obeject to move.
		**/
		ThreadSafe& operator=(ThreadSafe&& ts) {
			wrappedObj = std::move(ts.wrappedObj);
		}
		


		/**
		 * @brief The arrow operator is used to access members of the WrappedType object in a thread-safe way.
		 * @details A temporary object is returned. Such temporary object defines a `->` operator, which returns a pointer to the object wrapped in the ThreadSafe object who created it. Due to arrow operator concatenation rule of C++, this the operator is called whenever the `->` is called on the ThreadSafe object.
		 * @return An anonymous temporary object of type Temp, which holds a reference to this object, and locks the internal mutex on creation using a unique_lock.
		*/
		Temp operator->() {
			std::cout << "\x1B[31mThreadSafe ->\033[0m\n"; //DEBUG
			return Temp(*this);
		}


		/**
		 * @brief The dereference operator is used to get the instance of the wrapped in object in a thread-safe way.
		 * @details This operator allows to perform statements like: 
		 * @code{.cpp}
		 * thread_safe::ThreadSafe<std::string> threadSafeObject{"Hello world!"};
		 * std::string NOT_threadSafeObject = *safe;
		 * @endcode
		 * in a thread-safe way. 
		 * When a ThreadSafe object is dereferenced, a temporary object is returned. Such temporary object can be implicitly converted to WrappedType.
		 * @return An anonymous temporary object of type Temp, which holds a reference to this object, and locks the internal mutex on creation using a unique_lock.
		**/
		Temp operator*() {
			std::cout << "\x1B[31mThreadSafe *\033[0m\n"; //DEBUG
			return Temp(*this);
		}


		//TODO these 2 functions do the same thing, I have to choose one. The first one is more "elegant", the operator has to be before the object, but it needs to be between () (because it has less precedence than '.'). The second one needs no (), but it is more confusing
		/**
		 * @brief This operator is used to get the naked WrappedType object.
		 * @details Since `~` operator has a lower priority than mamber access operator (`.`), is almost always needed that the sub-expression `~threadSafeObject` is enclosed inside parentheses:
		 * @code
		 * thread_safe::ThreadSafe<std::string> threadSafeObject{"Hello"};
		 * (~threadSafeObject).append(" world!");
		 * @endcode
		 * @return 
		**/
		WrappedType& operator~() {
			std::cout << "\x1B[31mThreadSafe ~\033[0m\n"; //DEBUG
			return wrappedObj;
		}
		//TOREMOVE
		WrappedType& operator--(int) {
			std::cout << "\x1B[31mThreadSafe --\033[0m\n"; //DEBUG
			return wrappedObj;
		}



		/**
		 * @brief Locks the internal mutex and performs the function defined by the argument.
		 * @details The internal mutex is locked by a unique_lock. If inside the function the same ThreadSafe object (the lhs operand of this operator) is accessed throught `->`, `*` or `->*` operators, or appears in a comma separated list of ThreadSafe objects, a deadlock happens.
		 * Example of usage:
		 * @code
		 * thread_safe::ThreadSafe<std::string> threadSafeObject{"Hello"};
		 * int lenght = threadSafeObject ->* std::function([&threadSafeObject](){
		 *                                                    (~threadSafeObject).append(" world!"); 
         *                                                    return (~safe2).size();
		 *                                                });
		 * @endcode
		 * @tparam Return The return type of the callable.
		 * @param callable The actions to be performed after the lock of the internal mutex. Such function must have 0 arguments.
		 * @return The return of the callable.
		**/
		template<typename Return>
		Return operator->*(std::function<Return()> callable) {
			std::lock_guard<std::mutex>(mtx);
			return callable();
		}		


		//if two ThreadSafe objects are separated by a comma, locks those objects' mutexes and return the object representing the list of the locked mutexes

		/** TOREMOVE this comment
		 * @brief If a comma-separated list of locked mutexes and a ThreadSafe object is present, locks the new object's mutex and add it to the already existent list.
		 * @details The order of the argument (Locks, ThreadSafe<A>) is relevant, since the comma operator has a left-to-right associativity. This way it is possible to write expressions of the kind `SomeType obj = (ts1, ts2, ts3, ..., tsN)->*foobar(...);`. Note that the use of parentesis to enclose the comma separated list is necessary, since the comma is the least prioritized operator in C++.
		 * @tparam A The type of the object wrapped in
		 * @param locks The list of already locked mutexes
		 * @param ts The new ThreadSafe object to lock
		 * @return The new list of locked mutexes
		**/

		/**
		 * @brief locks the internal mutexes of a list of comma separated ThreadSafe objects.
		 * @details The order of evaluation of comma operator is left to right. The first pair of ThreadSafe objects of the list will call a specific overload of the `,` operator, which returns a newly constructed anonymous temporary object containing a list of unique_lock(s). The formal type of this object is Locks. The first list contains only the 2 unique_lock(s) relative to the internal mutexes of its arguments. The subsequent pairs of ThreadSafe objects will call another overload of `,` operator, which takes as arguments a Locks object (the one built by the previous `,` operator) and another ThreadSafe object. The internal mutex of this last ThreadSafe object is assigned to a unique_lock and added to the list of the Locks object. The lock object is anonymously returned, so it can be chained with another ThreadSafe object, and so on.
		 * @warning {deadlock} If the same ThreadSafe object appears multiple time on the list, a deadlock happens.
		 * @warning {unexpected behaviour} If the first N objects of a comma separated list are of type ThreadSafe, they will be merged into a Locks object and their internal mutexes will be locked, with potential unexpected behaviours.}
		**/
		///@{
		template <typename A, typename B>
		friend Locks operator,(ThreadSafe<A>& ts1, ThreadSafe<B>& ts2) {
			return Locks(std::unique_lock<std::mutex>(ts1.mtx), std::unique_lock<std::mutex>(ts2.mtx));
		}

		template <typename A>
		friend Locks operator,(Locks locks, ThreadSafe<A>& ts) {
			locks.add(std::unique_lock<std::mutex>(ts.mtx));
			return locks;
		}
		///@}

	};

}

#endif
