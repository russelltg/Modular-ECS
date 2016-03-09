/// \brief This defines the Manager class
/// \author Russell Greene

#pragma once

#include <boost/hana.hpp>

#include <vector>
#include <utility>
#include <bitset>
#include <type_traits>
#include <deque>
#include <cassert>
#include <iostream>

#include "SegmentedMap.h"

#include "MiscMetafunctions.h"
#include "Entity.h"


#undef max

/// \brief The templated class that holds custom storage for managers
/// Just override the template for your manager to use this!
template<typename T>
struct ManagerData{};

/// \brief Convenience function that creates a boost::hana::tuple of boost::hana::type_c<>s from a list of types
template<typename... T>
constexpr auto make_type_tuple = boost::hana::make_tuple(boost::hana::type_c<T>...);

/// we have to have these here because lambdas aren't allowed to be static, and because we need to have them in unevaluated contexts
namespace detail 
{
namespace lambdas
{
	
auto myStorageComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple, boost::hana::append(tuple, newElement));
	};
	
auto myTagComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), boost::hana::append(tuple, newElement), tuple);
	};
	
auto allStorageComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple, boost::hana::append(tuple, newElement));
	};
	
auto allTagComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), boost::hana::append(tuple, newElement), tuple);
	};
auto removeTypeAddVec = [](auto arg)
	{
		return std::vector<typename decltype(arg)::type>{};
	};
auto removeTypeAddSegmentedMap = [](auto arg)
	{
		return SegmentedMap<size_t, typename decltype(arg)::type>{};
	};
auto removeTypeAddPtr = [](auto arg)
	{
		return (typename decltype(arg)::type*){};
	};
auto getAllManagers = [](auto running, auto arg)
	{
		return boost::hana::concat(decltype(arg)::type::allManagers(), running);
	};
auto getAllComponents = [](auto arg)
	{
		return decltype(arg)::type::allComponents();
	};
}
}

/// \brief For distinguising if a class is a manager at all
struct ManagerBase{};


/// \brief The core class of the library; Defines components, 
template <typename Components_, typename Bases_ = boost::hana::tuple<> >
struct Manager : ManagerBase
{
	static_assert(decltype(is_tuple<Components_>())::value, "Components_ must be a boost::hana::tuple");
	static_assert(decltype(is_tuple<Bases_>())::value, "Bases_ must be a boost::hana::tuple");
	
	
	using myComponents_t = decltype(boost::hana::make<Components_>());
	
	/**
	 * @brief Gets the list of components owned by the manager. The size of this should be getNumMyComponents()
	 * 
	 * @return a boost::hana::tuple<> of components
	 */
	static constexpr auto myComponents() { return myComponents_t{}; }
	
	using myBases_t = decltype(boost::hana::make<Bases_>());
	/**
	 * @brief returns the direct bases of the manager. The size of this should be getNumBases()
	 * 
	 * @return a boost::hana::tuple<> of components
	 */
	static constexpr auto myBases() { return myBases_t{}; }
	
	using allManagers_t = decltype(boost::hana::append(remove_dups(boost::hana::concat(
		boost::hana::fold(myBases(), boost::hana::make_tuple(), detail::lambdas::getAllManagers), myBases())) , boost::hana::type_c<Manager>));
	/**
	 * @brief Returns all the managers that are accessable to this managers--in the order of all base managers (direct and indirect) then Manager (this manager class)
	 * 
	 * @return a boost::hana::tuple<> of all the accessable managers
	 */
	static constexpr auto allManagers() { return allManagers_t{}; }
	
	using allComponents_t = decltype(remove_dups(boost::hana::concat(boost::hana::fold(boost::hana::transform(myBases(), detail::lambdas::getAllComponents), 
		boost::hana::make_tuple(), boost::hana::concat), myComponents()))); 
	static constexpr auto allComponents() { return allComponents_t{}; }
	
	using myStorageComponents_t = decltype(boost::hana::fold(myComponents(), boost::hana::make_tuple(), detail::lambdas::myStorageComponents_LAM));
	static constexpr auto myStorageComponents() { return myStorageComponents_t{}; }
	
	using myTagComponents_t = decltype(boost::hana::fold(myComponents(), boost::hana::make_tuple(), detail::lambdas::myTagComponents_LAM));
	static constexpr auto myTagComponents() { return myTagComponents_t{}; }
	
	using allStorageComponents_t = decltype(boost::hana::fold(allComponents(), boost::hana::make_tuple(), detail::lambdas::allStorageComponents_LAM));
	static constexpr auto allStorageComponents() { return allStorageComponents_t{}; }

	using allTagComponents_t = decltype(boost::hana::fold(allComponents(), boost::hana::make_tuple(), detail::lambdas::allTagComponents_LAM));
	static constexpr auto allTagComponents() { return allTagComponents_t{}; }

	// STATIC CHECKS
	////////////////
	
	// make sure there are no duplicates in the components
	static_assert(decltype(boost::hana::size(myComponents()) == boost::hana::size(remove_dups(myComponents())))::value, "Don't pass the same component twice to the manager!");
	
	template<typename... Args>
	using TupleOfPtrs = std::tuple<Args*...>;

	// CONSTEXPR FUNCTIONS/TESTS


	using numComponents_t = decltype(boost::hana::size(allComponents()));
	static constexpr auto numComponents() { return numComponents_t{}; }
	
	using numMyComponents_t = decltype(boost::hana::size(myComponents()));
	static constexpr auto numMyComponents() { return numMyComponents_t{}; }
	
	template<typename T> 
	static constexpr auto isComponent(T componentToTest)
	{
		return boost::hana::contains(allComponents(), componentToTest);
	}
	template<typename T> 
	static constexpr auto isMyComponent(T componentToTest)
	{
		return boost::hana::contains(myComponents(), componentToTest);
	}
	template <typename T> 
	static constexpr auto getComponentID(T component)
	{
		return boost::hana::if_(isComponent(component), get_index_of_first_matching(allComponents(), component), boost::hana::nothing);
	}
	template <typename T> 
	static constexpr auto getMyComponentID(T component)
	{
		return boost::hana::if_(isComponent(component), get_index_of_first_matching(myComponents(), component), boost::hana::nothing);
	}

	using numStorageComponents_t = decltype(boost::hana::size(allStorageComponents()));
	static constexpr auto numStorageComponents() { return numStorageComponents_t{}; }
	
	using numMyStorageComponents_t = decltype(boost::hana::size(allStorageComponents()));
	static constexpr auto numMyStorageComponents() { return numMyStorageComponents_t{}; }
	
	template<typename T> 
	static constexpr auto isStorageComponent(T componentToTest)
	{
		return boost::hana::contains(allStorageComponents, componentToTest);
	}
	template<typename T> 
	static constexpr auto isMyStorageComponent(T componentToTest)
	{
		return boost::hana::contains(myStorageComponents(), componentToTest);
	}
	template <typename T> 
	static constexpr auto getMyStorageComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		return get_index_of_first_matching(myStorageComponents(), component);
	}
	template <typename T> 
	static constexpr auto getStorageComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		return get_index_of_first_matching(allStorageComponents(), component);
	}
	
	template<typename T> 
	static constexpr auto isTagComponent(T componentToTest)
	{
		return boost::hana::contains(allTagComponents(), componentToTest);
	}
	template<typename T> 
	static constexpr auto isMyTagComponent(T componentToTest)
	{
		return boost::hana::contains(myTagComponents(), componentToTest);
	}
	template <typename T> 
	static constexpr auto getTagComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isTagComponent(component));
		
		return get_index_of_first_matching(allTagComponents(), component);
	}
	
	using numManagers_t = decltype(boost::hana::size(allManagers()));
	static constexpr auto numManagers() { return numManagers_t{}; }
	
	template<typename T>
	static constexpr auto isManager(T toTest)
	{
	//	static_assert(std::is_base_of<ManagerBase, typename decltype(toTest)::type>::value, "Error, needs to be a manager");
		
		constexpr auto toTest_type = T{};
		
		return boost::hana::contains(allManagers(), toTest_type);
	}
	template <typename T> 
	static constexpr auto getManagerID(T manager)
	{
		BOOST_HANA_CONSTANT_CHECK(isManager(manager));  
		
		// force a constexpr context
		constexpr auto manager_type = T{};
		
		return get_index_of_first_matching(allManagers(), manager_type);
	}
	template<typename T> 
	static constexpr auto isBase(T baseToTest)
	{
		return boost::hana::contains(myBases(), baseToTest);
	}

	static constexpr auto numBases = boost::hana::size(myBases());
	
	template <typename T> 
	static constexpr auto getBaseID(T base)
	{
		BOOST_HANA_CONSTANT_CHECK(isBase(base));
		
		// to force a constexpr context
		constexpr auto base_type = T{};
		
		return get_index_of_first_matching(myBases(), base_type);
	}
	
	template<typename T>
	static constexpr auto isSignature(T signature)
	{
		return boost::hana::all_of(signature, [](auto type){ return isComponent(type); });
	}

	template<typename T>
	static constexpr auto getManagerFromComponent(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		return boost::hana::fold(allManagers(), boost::hana::type_c<boost::hana::none_t>, [component](auto last, auto toTest)
			{
				return boost::hana::if_(decltype(toTest)::type::isMyComponent(component), toTest, last);
			}
		);
	}


	template<typename T>
	static constexpr auto isolateStorageComponents(T toIsolate)
	{
		return boost::hana::fold(toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isStorageComponent(toTest), boost::hana::append(currentSet, toTest), currentSet);
			}
		);
	}
	template<typename T>
	static constexpr auto isolateTagComponents(T toIsolate)
	{
		return boost::hana::fold(toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isTagComponent(toTest), boost::hana::append(currentSet, toTest), currentSet);
			}
		);
	}
	template<typename T>
	static constexpr auto isolateMyComponents(T toIsolate)
	{
		return boost::hana::fold(toIsolate, boost::hana::make_set(), [](auto toTest, auto currentSet)
			{
				return boost::hana::if_(isMyComponent(toTest), boost::hana::append(currentSet, toTest), currentSet);
			}
		);
	}
	
	template<typename T>
	static constexpr auto findDirectBaseManagerForSignature(T signature)
	{
		
		return boost::hana::fold(myBases(), boost::hana::type_c<Manager>, [&signature](auto toTest, auto currentRet)
			{
				return boost::hana::if_(decltype(toTest)::type::isSignature(signature), toTest, currentRet);
			}
		);
	}
	
	template<typename T>
	static constexpr auto findMostBaseManagerForSignature(T signature)
	{
		using namespace boost::hana::literals;
		auto ret = boost::hana::while_([](auto pair){ return pair[0_c] != pair[1_c]; }, 
			boost::hana::make_tuple(boost::hana::type_c<Manager>, findDirectBaseManagerForSignature(signature)), [&signature](auto tup)
			{
				return boost::hana::make_tuple(tup[1_c], decltype(tup[0_c])::type::findDirectBaseManagerForSignature(signature)); 
			}
		);
		
		BOOST_HANA_CONSTANT_CHECK(isManager(ret[0_c]));
		return ret[0_c];
		
	}
	
	using RuntimeSignature_t = std::bitset<numComponents()>;

	template<typename T>
	static RuntimeSignature_t generateRuntimeSignature(T signature)
	{

		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));

		RuntimeSignature_t ret;

		boost::hana::for_each(signature, [&ret](auto type)
			{
				ret[decltype(getComponentID(type))::value] = true;
			}
		);

		return ret;
	}

	template<typename T, typename Components>
	auto& newEntity(T signature, Components&& components
		= decltype(components){} /*tuple of the components*/)
	{
		using namespace boost::hana::literals;
		
		entityStorage.emplace_back();
		const size_t newEntityIndex = entityStorage.size() - 1;
		Entity<Manager>& newEntityRef = entityStorage[newEntityIndex];
		newEntityRef.signature = generateRuntimeSignature(signature);
		newEntityRef.ID = newEntityIndex;
		newEntityRef.bases[boost::hana::size(newEntityRef.bases) - boost::hana::size_c<1>] = &newEntityRef;
		newEntityRef.destroy = [this, ID = newEntityRef.ID, signature]() 
			{
				// delete components
				boost::hana::for_each(isolateStorageComponents(signature), [this, ID](auto componentToDestroy)
					{
						getComponentStorage(componentToDestroy).erase(ID);
					});
				
				// delete entities
				boost::hana::for_each(entityStorage[ID].bases, [this, ID](auto basePtr)
					{
						constexpr auto baseType = std::remove_pointer_t<decltype(basePtr)>::managerType;
						BOOST_HANA_CONSTANT_CHECK(isManager(baseType));
						
						getRefToManager(baseType).freeEntitySlots.push_back(ID);
					});
			};
		
		// construct the components
		boost::hana::for_each(components, [this, newEntityIndex, &newEntityRef](auto& component) {
			constexpr auto component_type = boost::hana::type_c<std::decay_t<decltype(component)>>;
			BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component_type));
			
			constexpr auto manager_for_component = decltype(getManagerFromComponent(component_type)){};
			constexpr auto manager_id = getManagerID(manager_for_component);
			auto& refToManagerForComponent = getRefToManager(manager_for_component);
			
			auto& ptrToEntity = newEntityRef.bases[manager_id];
			if (ptrToEntity == nullptr)
			{
				refToManagerForComponent.entityStorage.emplace_back();
				size_t baseEntityID = refToManagerForComponent.entityStorage.size() - 1;
				auto& baseEntityRef = refToManagerForComponent.entityStorage[baseEntityID];
				baseEntityRef.ID = baseEntityID;
				baseEntityRef.bases[boost::hana::size(baseEntityRef.bases) - boost::hana::size_c<1>] = &baseEntityRef;
				baseEntityRef.destroy = newEntityRef.destroy;
				
				ptrToEntity = &baseEntityRef;
				
			}
			
			constexpr auto my_component_id = decltype(manager_for_component)::type::getMyStorageComponentID(component_type);
			constexpr auto all_component_id = decltype(manager_for_component)::type::getMyComponentID(component_type);
			
			refToManagerForComponent.storageComponentStorage[my_component_id][ptrToEntity->ID] = std::move(component);
			refToManagerForComponent.componentEntityStorage[decltype(all_component_id)::value].push_back(ptrToEntity->ID);			
		});
		
		return newEntityRef;
		
	}

	template<typename...Args>
	using TupleOfVectorRefrences = std::tuple<std::vector<Args>&...>;
	
	// returns the elements created [first, last)
	template<typename T, typename Components>
	std::pair<size_t, size_t> createEntityBatch(T signature, Components components, size_t numToConstruct)
	{
		// TODO: implement
	}
	void destroyEntity(Entity<Manager>* handle)
	{
		handle->destroy();
	}

	template<typename T>
	auto getStorageComponent(T component, Entity<Manager>* handle) -> typename decltype(component)::type&
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		constexpr auto managerForComponent = decltype(getManagerFromComponent(component)){};

		constexpr auto staticID = decltype(managerForComponent)::type::template getMyStorageComponentID(component);
		
		return getRefToManager(managerForComponent).storageComponentStorage[staticID][handle->ID];
		
	}

	template<typename T>
	bool hasComponent(T component, Entity<Manager>* entity)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		constexpr auto managerForComponent = decltype(getManagerFromComponent(component)){};

		auto ent = getEntityPtr(managerForComponent, entity);

		return ent->signature[decltype(decltype(managerForComponent)::type::template getComponentID(component))::value];
	}
	

	
	template<typename T>
	static auto getEntityPtr(T managerToGet, Entity<Manager>* ent) -> Entity<typename decltype(managerToGet)::type>*
	{
		BOOST_HANA_CONSTANT_CHECK(isManager(managerToGet));
		
		return ent->bases[getManagerID(managerToGet)];
	}
	
	template<typename T>
	auto getRefToManager(T manager) -> typename decltype(manager)::type&
	{
		BOOST_HANA_CONSTANT_CHECK(isManager(manager));
		
		return *basePtrStorage[getManagerID(manager)];
	}
	
	
	template<typename T>
	auto getComponentStorage(T component) -> SegmentedMap<size_t, typename decltype(component)::type>&
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		constexpr auto manager = decltype(getManagerFromComponent(component)){};
		
		const constexpr auto ID = decltype(manager)::type::template getMyStorageComponentID(component);
		
		return getRefToManager(manager).storageComponentStorage[ID];
		
	}


	template<typename T>
	std::vector<Entity<Manager>*>& getComponentEntityStorage(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		static constexpr auto manager = getManagerFromComponent(component);
		
		const constexpr auto ID = decltype(manager)::type::template getMyComponentID(component);
		
		return getRefToManager(manager).componentEntityStorage[ID];
	}

	// CALLING FUNCTIONS ON ENTITIES
	template<typename T, typename F>
	void callFunctionWithSigParams(Entity<Manager>* ent, T signature, F&& func)
	{
		// get components and put them in a tuple
		
		auto components = boost::hana::fold(signature, boost::hana::make_tuple(), [this, ent](auto retTuple, auto nextType) -> decltype(auto)
			{
				BOOST_HANA_CONSTANT_CHECK(isStorageComponent(nextType));
				return boost::hana::append(retTuple, getStorageComponent(nextType, ent));
			});
		
		// expand
		boost::hana::unpack(components, std::forward<F>(func));
	}

	template<typename T, typename F>
	void runAllMatching(T signature, F&& functor)
	{
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));
		
		static constexpr auto manager = decltype(findMostBaseManagerForSignature(signature)){};

		getRefToManager(manager).runAllMatchingIMPL(signature, std::forward<F>(functor));
	}

	template<typename T, typename F>
	void runAllMatchingIMPL(T signature, F&& functor)
	{
		using namespace boost::hana::literals;
		
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));
		
		// TODO: use shortest
		
		auto& entityVector = entityStorage;
		
		const auto runtimeSig = generateRuntimeSignature(signature);
		constexpr auto sigStorageCompsOnly = decltype(isolateStorageComponents(signature)){};
		
		for(Entity<Manager>& entity : entityVector)
		{
			if((runtimeSig & entity.signature) == runtimeSig) 
			{
				callFunctionWithSigParams(&entity, sigStorageCompsOnly, std::forward<F>(functor));
			}
		}
	}

	ManagerData<Manager> myManagerData;

	// storage for the actual components
	decltype(boost::hana::transform(myStorageComponents(), detail::lambdas::removeTypeAddSegmentedMap)) storageComponentStorage;
	std::array<std::vector<size_t>, numMyComponents()> componentEntityStorage;
	decltype(boost::hana::transform(allManagers(), detail::lambdas::removeTypeAddPtr)) basePtrStorage;
	std::vector<Entity<Manager>> entityStorage;
	std::deque<size_t> freeEntitySlots;
	
	bool hasBegunPlay = false;
	bool hasBeenCleandUp = false;
	
	size_t tickNumber = 0;
	
	ManagerData<Manager>& getManagerData()
	{
		return myManagerData;
	}


	Manager(const decltype(boost::hana::transform(myBases(), detail::lambdas::removeTypeAddPtr))& bases = {})
	{
		using namespace boost::hana::literals;
		
		// we don't need to assign this, it is just this!
		auto tempBases = boost::hana::drop_back(basePtrStorage);
		
		boost::hana::for_each(tempBases, [&bases](auto& baseToSet)
			{
			
				// get a hana type_c of the basetoset
				auto constexpr baseToSet_type = boost::hana::type_c<std::remove_pointer_t<std::decay_t<decltype(baseToSet)>>>;
				BOOST_HANA_CONSTANT_CHECK(isManager(baseToSet_type));
			
				// a lambda that checks if a contains the base we want
				auto hasBase = [&baseToSet_type](auto typeToCheck)
				{
					return decltype(typeToCheck)::type::isManager(baseToSet_type);
				};
				
				constexpr auto directBaseThatHasPtr_opt = decltype(boost::hana::find_if(myBases(), hasBase)){};
				BOOST_HANA_CONSTANT_CHECK(boost::hana::is_just(directBaseThatHasPtr_opt));
				
				constexpr auto directBaseThatHasPtr = *directBaseThatHasPtr_opt;
				BOOST_HANA_CONSTANT_CHECK(isBase(directBaseThatHasPtr));
				
				constexpr auto directBaseThatHasPtrID = decltype(Manager::getBaseID(directBaseThatHasPtr)){};
				
				baseToSet = &(bases[directBaseThatHasPtrID]->getRefToManager(baseToSet_type));
				
				if(!baseToSet)
				{
					std::cerr << "Could not find base: " << typeid(baseToSet).name() << 
						"; Did you forget to add it in the constructor?" << std::endl;
					std::terminate();
				}
			});
		
		
		basePtrStorage = boost::hana::append(tempBases, this);
		
	}

	~Manager()
	{
		// TODO: add callbacks
	}
};
