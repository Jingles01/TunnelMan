#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <vector>
#include <string>
#include <list>

const int OIL_FIELD_WIDTH = 64;
const int EARTH_FIELD_HEIGHT = 60;
const int GAME_BOARD_HEIGHT = 64;

const int TUNNEL_SHAFT_X_START = 30;
const int TUNNEL_SHAFT_X_END = 33;
const int TUNNEL_SHAFT_Y_TOP = 59;
const int TUNNEL_SHAFT_Y_BOTTOM_NO_EARTH = 4;

class StudentWorld : public GameWorld {
public:
    StudentWorld(std::string assetPath);
    virtual ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();

    bool removeEarth(int x, int y);
    bool isEarthAt(int x, int y) const;
    bool isEarthBelowBoulder(int x_boulder_left, int y_boulder_bottom) const;
    bool isEarthUnderneath4x4(int x_topLeft, int y_topLeft) const;

    bool isBoulderAtLocation(int x, int y, double checkRadius) const;
    bool isBoulderBlockingTunnelMan(int targetManX, int targetManY) const;

    bool canSquirtExistAt(int x, int y) const;

    void addActor(Actor* actor);
    TunnelMan* getTunnelMan() const { return m_tunnelman; }
    void revealNearbyObjects(int centerX, int centerY, double radius);
    bool annoyProtestersInRadius(Actor* instigator, int centerX, int centerY, double radius, int damage);
    void damageActorsInRadius(Actor* instigatorBoulder, int centerX, int centerY, double radius, int damage);
    bool checkAndHandleProtesterGoldPickup(Gold* nugget, int goldX, int goldY);
    bool wasAnnoyanceSourceBoulder(Actor* annoyedActor) const;

    void decrementBarrelsRemaining();

    bool canProtesterMoveTo(const Protester* protester, int targetX, int targetY) const;
    bool hasClearPathToTunnelMan(const Protester* protester, int startX, int startY, Actor::Direction dir, int& dx_to_tm, int& dy_to_tm, int& path_dist) const;
    Actor::Direction getPathToExit(int startX, int startY);
    Actor::Direction getPathToCoordinate(int startX, int startY, int endX, int endY);
    int getPathDistanceToCoordinate(int startX, int startY, int endX, int endY);
    double distance(int x1, int y1, int x2, int y2) const;

private:
    Earth* m_earth[OIL_FIELD_WIDTH][EARTH_FIELD_HEIGHT];
    TunnelMan* m_tunnelman;
    std::list<Actor*> m_actors;

    int m_barrelsRemaining;
    int m_ticksSinceLastProtesterAdded;
    int m_targetNumberOfProtesters;
    int m_currentNumberOfProtestersOnField;

    Actor* m_lastAnnoyanceSource;

    void populateOilFieldWithObjects();
    void removeDeadActors();
    void addNewActorsDuringTick();
    void updateGameStatText();
    bool isAnyObjectNearby(int x, int y, double radius, bool checkOnlyBoulders) const;

};

#endif // STUDENTWORLD_H_
