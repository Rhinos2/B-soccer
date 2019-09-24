#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H


#include <map>
//#include <assert>


class BaseGameEntity;

//provide easy access
#define EntityMgr EntityManager::Instance()



class EntityManager
{
private:

  //typedef std::map<int, BaseGameEntity*> std::map<int, BaseGameEntity*>;

private:

  //to facilitate quick lookup the entities are stored in a std::map, in which
  //pointers to entities are cross referenced by their identifying number
  std::map<int, BaseGameEntity*> m_EntityMap;

  EntityManager(){}

  //copy ctor and assignment should be private
  EntityManager(const EntityManager&);
  EntityManager& operator=(const EntityManager&);

public:

  static EntityManager* Instance();

  //this method stores a pointer to the entity in the std::vector
  //m_Entities at the index position indicated by the entity's ID
  //(makes for faster access)
  void            RegisterEntity(BaseGameEntity* NewEntity);

  //returns a pointer to the entity with the ID given as a parameter
  BaseGameEntity* GetEntityFromID(int id)const;

  //this method removes the entity from the list
  void            RemoveEntity(BaseGameEntity* pEntity);

  //clears all entities from the entity map
  void            Reset(){m_EntityMap.clear();}
};







#endif