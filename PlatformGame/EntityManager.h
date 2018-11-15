#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "j1Module.h"
#include "p2Animation.h"
#include "p2DynArray.h"

enum EntityType
{
	PLAYER,
	BAT,
	SPIDER,

	NO_TYPE
};

class Entity;
class Player;

class EntityManager : public j1Module
{
public:

	EntityManager();
	~EntityManager();

	bool Awake(pugi::xml_node& conf);

	bool Start();

	bool PreUpdate();

	bool Update(float dt);

	bool PostUpdate();

	bool Load(pugi::xml_node&);

	bool Save(pugi::xml_node&) const;

	bool CleanUp();

	void OnCollision(Collider* c1, Collider* c2);

	Entity* CreateEntity(EntityType type);

	Player* GetPlayerData() const;

private:

	p2DynArray<Entity*> entities;

public:

	Entity* player;
	bool ActivePlayer = false;
	fPoint positionStartMap1;
	fPoint positionStartMap2;
};

#endif
