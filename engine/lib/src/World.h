#ifndef __World_h__
#define __World_h__

#include "GameObject.h"
#include <unordered_map>

namespace Galaxy3D
{
	class World
	{
	public:
		//ֻ��World����GameObject��shared_ptr������һ��ʹ��weak_ptr
		static void AddGameObject(const std::shared_ptr<GameObject> &obj);
		static std::weak_ptr<GameObject> FindGameObject(GameObject *obj);
		static void Init();
		static void Update();
		static void Done();
		static void OnPause();
		static void OnResume();

	private:
		static std::unordered_map<GameObject *, std::shared_ptr<GameObject>> m_gameobjects;
		static std::unordered_map<GameObject *, std::shared_ptr<GameObject>> m_gameobjects_new;
	};
}

#endif