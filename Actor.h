#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"

class StudentWorld;


class Actor : public GraphObject {
public:
    Actor(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, StudentWorld* world, int initialHP = 0, bool initiallyVisible = true);
    virtual ~Actor();
    virtual void doSomething() = 0;

    virtual bool annoy(int damagePoints);

    bool isAlive() const;
    void setDead();
    int getHP() const;
    void setHP(int hp);

    StudentWorld* getWorld() const;

    virtual bool blocksMovement() const;
    virtual bool canBeHit() const;
    virtual bool canBeBonked() const;
    virtual bool canPickUpGold() const;
    virtual bool isDamageable() const;
    void setVisibleWithCheck(bool visible);

private:
    StudentWorld* world;
    bool alive;
    int hp;
};

class TunnelMan : public Actor {
public:
    TunnelMan(StudentWorld* world);
    virtual ~TunnelMan();

    virtual void doSomething() override;
    virtual bool annoy(int damagePoints) override;
    virtual bool blocksMovement() const override;
    virtual bool canBeBonked() const override;
    virtual bool isDamageable() const override;

    void addGold(int amount = 1);
    void useGold();
    int getGoldCount() const;

    void addWater(int amount = 5);
    void useSquirt();
    int getWaterCount() const;

    void addSonar(int amount = 2);
    void useSonar();
    int getSonarCount() const;

private:
    int squirts;
    int sonar;
    int gold;
};

class Earth : public Actor {
public:
    Earth(StudentWorld* world, int startX, int startY);
    virtual ~Earth();

    virtual void doSomething() override;
    virtual bool blocksMovement() const override;
    virtual bool annoy(int damagePoints) override;
};

class Boulder : public Actor {
public:
    enum class State { STABLE, WAITING, FALLING };

    Boulder(StudentWorld* world, int startX, int startY);
    virtual ~Boulder();

    virtual void doSomething() override;
    virtual bool blocksMovement() const override;
    virtual bool annoy(int damagePoints) override;

private:
    State state;
    int waitingTicks;
    void clearEarth();
};

class Squirt : public Actor {
public:
    Squirt(StudentWorld* world, int startX, int startY, Direction dir);
    virtual ~Squirt();

    virtual void doSomething() override;
    virtual bool blocksMovement() const override;
    virtual bool annoy(int damagePoints) override;

private:
    int remainingDistance;
};

class Goodie : public Actor {
public:
    Goodie(int imageID, int startX, int startY, StudentWorld* world, int points, int initialHP = 0, bool initiallyVisible = false);
    virtual ~Goodie();

    virtual void doSomething() override;
    virtual bool annoy(int damagePoints) override;
    virtual bool canBePickedUpByTunnelMan() const;
    int getPoints() const;
    virtual void activate(TunnelMan* tunnelman) = 0;

private:
    int points;
};

class BarrelOfOil : public Goodie {
public:
    BarrelOfOil(StudentWorld* world, int startX, int startY);
    virtual ~BarrelOfOil();
    virtual void activate(TunnelMan* tunnelman) override;
};

class Gold : public Goodie {
public:
    enum class State { PERMANENT_FOR_TUNNELMAN, TEMPORARY_FOR_PROTESTER };

    Gold(StudentWorld* world, int startX, int startY);
    Gold(StudentWorld* world, int startX, int startY, bool isTemporary);
    virtual ~Gold();

    virtual void doSomething() override;
    virtual bool canBePickedUpByProtester() const;

    void setPickedUpByProtester(bool pickedUp);
    bool wasPickedUpByProtester() const;
    virtual void activate(TunnelMan* tunnelman) override;

    State getGoldState() const { return goldState; }

private:
    State goldState;
    int totalTicks;
    bool pickedUpByProtester;
};

class TemporaryGoodie : public Goodie {
public:
    TemporaryGoodie(int imageID, int startX, int startY, StudentWorld* world, int points, int lifetime);
    virtual ~TemporaryGoodie();
    virtual void doSomething() override;

private:
    int totalTicksRemaining;
};

class SonarKit : public TemporaryGoodie {
public:
    SonarKit(StudentWorld* world, int startX, int startY, int lifetime);
    virtual ~SonarKit();

protected:
    virtual void activate(TunnelMan* tunnelman) override;
};

class WaterPool : public TemporaryGoodie {
public:
    WaterPool(StudentWorld* world, int startX, int startY, int lifetime);
    virtual ~WaterPool();

protected:
    virtual void activate(TunnelMan* tunnelman) override;
};

class Protester : public Actor {
public:
    Protester(int imageID, StudentWorld* world, int initialHP);
    virtual ~Protester();

    virtual void doSomething() override;
    virtual bool annoy(int damagePoints) override;
    virtual bool blocksMovement() const override;
    virtual bool canBeHit() const override;
    virtual bool canBeBonked() const override;
    virtual bool canPickUpGold() const override;
    virtual bool isDamageable() const override;

    void setMustLeaveOilField();
    bool mustLeaveOilField() const;

    virtual void acceptGold() = 0;

protected:
    bool isResting() const;
    void decrementRestingTicks();
    void setRestingTicks(int ticks);
    bool attemptToShout();
    bool lineOfSightToTunnelman(int& dx, int& dy, int& distance);
    bool moveTowards(int targetX, int targetY);
    void pickNewRandomDirectionAndSteps();
    bool canMoveInDirection(Direction dir) const;

    Direction getNextMoveToExit();
    Direction getNextMoveToTunnelMan(int targetX, int targetY);

    int ticksToWaitBetweenMoves;
    int restingTicks;
    int numSquaresToMoveInCurrentDirection;
    bool mustLeave;
    int ticksSinceLastShout;
    int ticksSinceLastPerpendicularTurn;
};

class RegularProtester : public Protester {
public:
    RegularProtester(StudentWorld* world, int initialHP);
    virtual ~RegularProtester();

    virtual void doSomething() override;
    virtual void acceptGold() override;
    virtual bool annoy(int damagePoints) override;
};

class HardcoreProtester : public Protester {
public:
    HardcoreProtester(StudentWorld* world, int initialHP);
    virtual ~HardcoreProtester();

    virtual void doSomething() override;
    virtual void acceptGold() override;
    virtual bool annoy(int damagePoints) override;

private:
    int ticksToStareAtGold;
};

#endif // ACTOR_H_
