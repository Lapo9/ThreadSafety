#ifndef THREAD_SAFE_OBJECT
#define THREAD_SAFE_OBJECT

#include <mutex>
#include <functional>
#include <vector>
#define __cpp_lib_concepts
#include <concepts>
#include <iostream> //DEBUG

//C++20 templates needs to be restricted by concepts (they are available in MSVS2019 preview 16.6 v3)
namespace thread_safe {

	template<typename WrappedType>
	class ThreadSafe; //forward declaration

	//C++20 this class should not be visible from outside, but it cannot be a nested class of ThreadSafe (because it is a template class), so maybe it can be NOT exported(?)
	//TODO there is the big problem that this mechanism is not deadlock safe. Example: 2 different threads run at the same time (ts1, ts2, ts3)->*... and (ts3, ts2, ts1)->*... The first thread locks ts1, the second thread locks ts3, the first locks ts2, the second tries to lock ts2 (but cannot), the first triest to lock ts3 (but cannot). A way to solve this is to make a thread release all of the mutex acquired in the comma separated list as soon as it fails to lock a mutex (and retry immediately), but there is the risk that the same situation goes on forever! Maybe it is possible to implement an algorithm similiar to the CSMACA (for wifi) but it would be slow (I think).
	//This object contains an array of unique_locks. It is instantiated temporarily, so that all of the ThreadSafe objects in a comma separated list are locked for the duration of the statement.
	class LocksList {
		template<typename WrappedType> friend class ThreadSafe; //ThreadSafe must be the only class able to create and interact with a LocksList object.
		using unique_lock = std::unique_lock<std::mutex>;
		std::vector<unique_lock> lockGuards; //The list of the locked mutexes (throughout unique_lock) guarded by this object.

		//Constructs a LocksList object made up of 2 unique_lock(s) guarding the internal mutexes of the ThreadSafe objects passed as arguments.
		template <typename A, typename B>
		LocksList(ThreadSafe<A>& ts1, ThreadSafe<B>& ts2) {
			lockGuards.push_back(unique_lock(ts1.mtx));
			lockGuards.push_back(unique_lock(ts2.mtx));
		}

		//Adds a new unique_lock, guarding the internal mutex of the ThreadSafe object passed as argument, to the list
		template <typename A>
		void add(ThreadSafe<A>& ts) {
			lockGuards.push_back(unique_lock(ts.mtx));
		}


		//THINK is this necessary? If someone wants to do more than one thing after the LocksList he can easyly wrap all of the instructions in a lambda and subsequently call it. Example: [...]()->ReturnType{...} () (look at the example in MainTEST/scratch
		/*
		 * @brief The `->*` operator is used to chain a function call after a LocksList list.
		 * @details The callable must be a function with 0 arguments. Inside such callable, any function can be executed with safe access to the ThreadSafe objects mentioned in the left-hand-side list. 
		 * If inside the callable, at any point, one of the ThreadSafe objects mentioned in the lhs list is accessed through `->` or `*` operators, or is mentioned in a comma separated list preceded only by ThreadSafe objects (or as first item of such list), a deadlock will occur.
		 * The first parameter is not used at all, but it is necessary to call this overload only when the operator has a LocksList as lhs.
		 * @tparam Return The same return type of the callable.
		 * @param callable The function with 0 arguments to call.
		 * @return Return
		**/
//		template<typename Return>
//		friend Return&& operator->*(const LocksList&, std::function<Return()> callable) {
//			std::cout << "\x1B[31mLocksList ->* (const LocksList&, std::function<Return()>)\033[0m\n"; //DEBUG
//			return std::forward<Return>(callable());
//		}

		
		/**
		 * @brief The `->*` operator is used to chain an operation after a LocksList list.
		 * @details Any can be executed with safe access to the ThreadSafe objects mentioned in the left-hand-side list. 
		 * If during the operation, at any point, one of the ThreadSafe objects mentioned in the lhs list is accessed through `->` or `*` operators, or is mentioned in a comma separated list (preceded only by ThreadSafe objects or as first item of such list), a deadlock will occur.
		 * The first parameter is not used at all, but it is necessary to call this overload only when the operator has a LocksList as lhs.
		 * @tparam Return The same return type of the callable.
		 * @param callable The function with 0 arguments to call.
		 * @return Return
		**/
		template<typename Return>
		friend Return&& operator->*(const LocksList&, Return&& ret) {
			std::cout << "\x1B[31mLocksList ->* (const LocksList&, Return&&)\033[0m\n"; //DEBUG
			return std::forward<Return>(ret);
		}


		//Comma operators are declared here because they need to be friend with both LocksList and ThreadSafe. This is because I'd like LocksList objects to expose no methods, since they are just an artifact to group more ThreadSafe objects present in a comma separated list.
		template <typename A, typename B>
		friend LocksList operator,(ThreadSafe<A>& ts1, ThreadSafe<B>& ts2);

		template <typename A>
		friend LocksList operator,(LocksList locks, ThreadSafe<A>& ts);
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

		//The temporary class instantiated each time an object of type ThreadSafe is accessed. The object is destroyed at the end of the full expression where it has been accessed.
		class Temp {
			ThreadSafe& real; //The reference to the permanent object.
			std::lock_guard<std::mutex> guard {real.mtx};


			public:
			//Constructs a Temp object given a ThreadSafe reference. This is the only way to build a Temp object.
			Temp(ThreadSafe& real) : real{real} {}

			//Returns the object wrapped in the ThreadSafe object used to build this Temp Object. A pointer is returned because `->` needs a pointer as return type.
			WrappedType* operator->() {
				std::cout << "\x1B[31mTemp ->\033[0m\n"; //DEBUG
				return &real.wrappedObj;
			}

			//Converts the Temp object to the WrappedType of the ThreadSafe object used to constructs this Temp object.
			operator WrappedType&() {
				std::cout << "\x1B[31mTemp ->\033[0m\n"; //DEBUG
				return real.wrappedObj;
			}

		};

		friend class LocksList; //LocksList objects needs to access some private members of ThreadSafe objects (in particular mtx).


		WrappedType wrappedObj; //Object to wrap into this ThreadSafe object
		std::mutex mtx; //Internal mutex associated with the wrappedObj


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
		 * @code
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


		//TODO update documentation comment
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
		Return&& operator->*(Return&& ret) {
			std::cout << "\x1B[31mThreadSafe ->*\033[0m\n"; //DEBUG
			std::unique_lock<std::mutex> guard(mtx);
			return std::forward<Return>(ret);
		}	
	


		//Comma operators are declared here because they need to be friend with both LocksList and ThreadSafe.
		template <typename A, typename B>
		friend LocksList operator,(ThreadSafe<A>& ts1, ThreadSafe<B>& ts2);

		template <typename A>
		friend LocksList operator,(LocksList locks, ThreadSafe<A>& ts);

	};


	
	//TODO args should be const references (and mtx mutable)?
	/**
		 * @name Comma Operator
		 * @brief LocksList the internal mutexes of a list of comma separated ThreadSafe objects.
		 * @details The order of evaluation of comma operator is left to right. The first pair of ThreadSafe objects of the list will call a specific overload of the `,` operator,					which	returns a newly constructed anonymous temporary object containing a list of unique_lock(s). The formal type of this object is LocksList. The first list			contains only       the 2	  unique_lock(s) relative to the internal mutexes of its arguments. The subsequent pairs of ThreadSafe objects will call another overload of `,`		operator, which		takes as arguments  a LocksList object (the one built by the previous `,` operator) and another ThreadSafe object. The internal mutex of this last		ThreadSafe object is		assigned to a unique_lock	 and added to the list of the LocksList object. The lock object is anonymously returned, so it can be chained	with	another ThreadSafe object, and		  so on. For example:
		 * @code
		 *    (ts1, ts2, ts3, ts4);
		 * // --------->called: operator,(ThreadSafe<A>&, ThreadSafe<B>&)
		 *    (locks,    ts3, ts4);
		 * // -------------->called: operator,(LocksList, ThreadSafe<A>&)
		 *    (locks,         ts4);
		 * // ------------------->called: operator,(LocksList, ThreadSafe<A>&)
		 *     locks;
		 * @endcode
		 * @warning **Deadlock** If the same ThreadSafe object appears multiple time on the list, a deadlock happens.
		 * @warning **Unexpected behaviour** If the first N objects of a comma separated list are of type ThreadSafe, they will be merged into a LocksList object and their internal				mutexes	 will be locked, with potential unexpected behaviours.
		**/
	///@{
	/**
		 * @brief This overload is invoked on the first pair of ThreadSafe objects in a list of comma separated ThreadSafe objects.
		 * @tparam A The type of the object wrapped in the first ThreadSafe argument.
		 * @tparam B The type of the object wrapped in the second ThreadSafe argument.
		 * @param ts1 The first ThreadSafe object whose internal mutex is to be locked.
		 * @param ts2 The second ThreadSafe object whose internal mutex is to be locked.
		 * @return A newly constructed anonymous LocksList object, whose internal list contains the unique_lock(s) relative to the the internal mutexes of the arguments.
		**/
	template <typename A, typename B>
	LocksList operator,(ThreadSafe<A>& ts1, ThreadSafe<B>& ts2) {
		std::cout << "\x1B[31mThreadSafe ,\033[0m\n"; //DEBUG
		return LocksList{ts1, ts2};
	}

	/**
	 * @brief This overload is invoked on a list of comma separated ThreadSafe object, apart from the first pair.
	 * @tparam A The type of the object wrapped in the ThreadSafe object to lock.
	 * @param locks The object containing the list of already locked mutexes Uunique_locks(s)).
	 * @param ts The ThreadSafe object to add to the list of locks.
	 * @return The locks object with the new lock added to the internal list.
	**/
	template <typename A>
	LocksList operator,(LocksList locks, ThreadSafe<A>& ts) {
		std::cout << "\x1B[31mLocksList ,\033[0m\n"; //DEBUG
		locks.add(ts);
		return locks;
	}
	///@}






	//TEST it doesn't work!
	template <std::invocable<void> Callable>
	auto chainTEST(const LocksList&, Callable callable) {
		return std::forward<auto>(callable());
	}
}

#endif
