#ifndef __SCENE_H__
#define __SCENE_H__

class Scene
{
public:
	Scene();
	virtual ~Scene();
	virtual void Update() = 0;
	virtual void Draw() = 0;
};

#endif // __SCENE_H__