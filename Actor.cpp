#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <algorithm>
#include <vector>
using namespace std;


Actor::Actor(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, StudentWorld* world_ptr, int initialHP, bool initiallyVisible)
    : GraphObject(imageID, startX, startY, dir, size, depth),
      world(world_ptr), alive(true), hp(initialHP) {
    if (initiallyVisible) {
        setVisibleWithCheck(true);
    }
}

Actor::~Actor() {
}

bool Actor::annoy(int damagePoints) {
    if (!isAlive()) return false;

    hp -= damagePoints;
    if (hp <= 0) {
        hp = 0;
        setDead();
        return true;
    }
    return false;
}

bool Actor::isAlive() const {
    return alive;
}

void Actor::setDead() {
    alive = false;
}

int Actor::getHP() const {
    return hp;
}

void Actor::setHP(int newHp) {
    hp = newHp;
    if (hp <= 0 && alive) {
        hp = 0;
        setDead();
    }
}

StudentWorld* Actor::getWorld() const {
    return world;
}

bool Actor::blocksMovement() const { return false; }
bool Actor::canBeHit() const { return false; }
bool Actor::canBeBonked() const { return false; }
bool Actor::canPickUpGold() const { return false; }
bool Actor::isDamageable() const { return false; }

void Actor::setVisibleWithCheck(bool visible_status) {
    GraphObject::setVisible(visible_status);
}




TunnelMan::TunnelMan(StudentWorld* worldPtr)
    : Actor(TID_PLAYER, 30, 60, right, 1.0, 0, worldPtr, 10, true),
      squirts(5), sonar(1), gold(0) {
}

TunnelMan::~TunnelMan() {
}

void TunnelMan::doSomething() {
    if (!isAlive()) {
        return;
    }

    bool dugThisTick = false;
    for (int i = 0; i < SPRITE_WIDTH; ++i) {
        for (int j = 0; j < SPRITE_HEIGHT; ++j) {
            if (getWorld()->removeEarth(getX() + i, getY() + j)) {
                dugThisTick = true;
            }
        }
    }
    if (dugThisTick) {
        getWorld()->playSound(SOUND_DIG);
    }

    int ch;
    if (getWorld()->getKey(ch)) {
        switch (ch) {
            case KEY_PRESS_LEFT:
                if (getDirection() == left) {
                    if (getX() > 0 && !getWorld()->isBoulderBlockingTunnelMan(getX() - 1, getY())) {
                        moveTo(getX() - 1, getY());
                    }
                } else {
                    setDirection(left);
                }
                break;
            case KEY_PRESS_RIGHT:
                if (getDirection() == right) {
                    if (getX() < VIEW_WIDTH - SPRITE_WIDTH && !getWorld()->isBoulderBlockingTunnelMan(getX() + 1, getY())) {
                        moveTo(getX() + 1, getY());
                    }
                } else {
                    setDirection(right);
                }
                break;
            case KEY_PRESS_UP:
                if (getDirection() == up) {
                    if (getY() < EARTH_FIELD_HEIGHT && !getWorld()->isBoulderBlockingTunnelMan(getX(), getY() + 1)) {
                        moveTo(getX(), getY() + 1);
                    }
                } else {
                    setDirection(up);
                }
                break;
            case KEY_PRESS_DOWN:
                if (getDirection() == down) {
                    if (getY() > 0 && !getWorld()->isBoulderBlockingTunnelMan(getX(), getY() - 1)) {
                        moveTo(getX(), getY() - 1);
                    }
                } else {
                    setDirection(down);
                }
                break;
            case KEY_PRESS_ESCAPE:
                setHP(0);
                break;
            case KEY_PRESS_SPACE:
                if (squirts > 0) {
                    useSquirt();
                    getWorld()->playSound(SOUND_PLAYER_SQUIRT);
                    int squirtX = getX(), squirtY = getY();
                    if (getDirection() == up) squirtY += SPRITE_HEIGHT;
                    else if (getDirection() == down) squirtY -= SPRITE_HEIGHT;
                    else if (getDirection() == left) squirtX -= SPRITE_WIDTH;
                    else if (getDirection() == right) squirtX += SPRITE_WIDTH;

                    if (getWorld()->canSquirtExistAt(squirtX, squirtY)) {
                         getWorld()->addActor(new Squirt(getWorld(), squirtX, squirtY, getDirection()));
                    }
                }
                break;
            case 'z':
            case 'Z':
                if (sonar > 0) {
                    useSonar();
                    getWorld()->playSound(SOUND_SONAR);
                    getWorld()->revealNearbyObjects(getX(), getY(), 12);
                }
                break;
            case KEY_PRESS_TAB:
                if (gold > 0) {
                    useGold();
                    getWorld()->addActor(new Gold(getWorld(), getX(), getY(), true));
                }
                break;
        }
    }
}

bool TunnelMan::annoy(int damagePoints) {
    if (Actor::annoy(damagePoints)) {
        getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
        return true;
    }
    return false;
}

bool TunnelMan::blocksMovement() const {
    return false;
}

bool TunnelMan::canBeBonked() const {
    return true;
}
bool TunnelMan::isDamageable() const {
    return true;
}

void TunnelMan::addGold(int amount) {
    gold += amount;
}

void TunnelMan::useGold() {
    if (gold > 0) gold--;
}

int TunnelMan::getGoldCount() const {
    return gold;
}

void TunnelMan::addWater(int amount) {
    squirts += amount;
}

void TunnelMan::useSquirt() {
    if (squirts > 0) squirts--;
}

int TunnelMan::getWaterCount() const {
    return squirts;
}

void TunnelMan::addSonar(int amount) {
    sonar += amount;
}

void TunnelMan::useSonar() {
    if (sonar > 0) sonar--;
}

int TunnelMan::getSonarCount() const {
    return sonar;
}


//==================================================================================================
// Earth Implementations
//==================================================================================================

Earth::Earth(StudentWorld* world_ptr, int startX, int startY)
    : Actor(TID_EARTH, startX, startY, right, 0.25, 3, world_ptr, 0, true) {
}

Earth::~Earth() {}

void Earth::doSomething() {
}

bool Earth::blocksMovement() const {
    return true;
}

bool Earth::annoy(int damagePoints) {
    return false;
}

//==================================================================================================
// Boulder Implementations
//==================================================================================================

Boulder::Boulder(StudentWorld* world_ptr, int startX, int startY)
    : Actor(TID_BOULDER, startX, startY, down, 1.0, 1, world_ptr, 0, true),
      state(State::STABLE), waitingTicks(0) {
    clearEarth();
}

Boulder::~Boulder() {}

void Boulder::clearEarth() {
    for (int i = 0; i < SPRITE_WIDTH; ++i) {
        for (int j = 0; j < SPRITE_HEIGHT; ++j) {
            getWorld()->removeEarth(getX() + i, getY() + j);
        }
    }
}

void Boulder::doSomething() {
    if (!isAlive()) {
        return;
    }

    switch (state) {
        case State::STABLE:
            if (!getWorld()->isEarthBelowBoulder(getX(), getY())) {
                state = State::WAITING;
                waitingTicks = 30;
            }
            break;
        case State::WAITING:
            waitingTicks--;
            if (waitingTicks <= 0) {
                state = State::FALLING;
                getWorld()->playSound(SOUND_FALLING_ROCK);
            }
            break;
        case State::FALLING:
            {
                int targetY = getY() - 1;
                if (targetY < 0 || getWorld()->isEarthUnderneath4x4(getX(), targetY) || getWorld()->isBoulderAtLocation(getX(), targetY, 0.0)) {
                    setDead();
                } else {
                    moveTo(getX(), targetY);
                    getWorld()->damageActorsInRadius(this, getX(), getY(), 3.0, 100);
                }
            }
            break;
    }
}

bool Boulder::blocksMovement() const {
    return true;
}

bool Boulder::annoy(int damagePoints) {
    return false;
}


//==================================================================================================
// Squirt Implementations
//==================================================================================================
Squirt::Squirt(StudentWorld* world_ptr, int startX, int startY, Direction dir)
    : Actor(TID_WATER_SPURT, startX, startY, dir, 1.0, 1, world_ptr, 0, true),
      remainingDistance(4) {}

Squirt::~Squirt() {}

void Squirt::doSomething() {
    if (!isAlive()) return;

    if (getWorld()->annoyProtestersInRadius(this, getX(), getY(), 3.0, 2)) {
        setDead();
        return;
    }

    remainingDistance--;
    if (remainingDistance < 0) {
        setDead();
        return;
    }

    int nextX = getX();
    int nextY = getY();
    switch (getDirection()) {
        case up:    nextY++; break;
        case down:  nextY--; break;
        case left:  nextX--; break;
        case right: nextX++; break;
        default: setDead(); return;
    }

    if (!getWorld()->canSquirtExistAt(nextX, nextY)) {
        setDead();
        return;
    }
    moveTo(nextX, nextY);
}

bool Squirt::blocksMovement() const { return false; }
bool Squirt::annoy(int damagePoints) { return false; }


//==================================================================================================
// Goodie Base Class Implementations
//==================================================================================================
Goodie::Goodie(int imageID, int startX, int startY, StudentWorld* world_ptr, int point_value, int initialHP, bool initiallyVisible)
    : Actor(imageID, startX, startY, right, 1.0, 2, world_ptr, initialHP, initiallyVisible), points(point_value) {}

Goodie::~Goodie() {}

void Goodie::doSomething() {
    if (!isAlive()) return;

    TunnelMan* tm = getWorld()->getTunnelMan();
    if (tm == nullptr || !tm->isAlive()) return;

    if (!isVisible() && getWorld()->distance(getX(), getY(), tm->getX(), tm->getY()) <= 4.0) {
        setVisibleWithCheck(true);
        return;
    }

    if (isVisible() && canBePickedUpByTunnelMan() && getWorld()->distance(getX(), getY(), tm->getX(), tm->getY()) <= 3.0) {
        setDead();
        getWorld()->increaseScore(points);
        activate(tm);
    }
}
bool Goodie::annoy(int damagePoints) { return false; }

bool Goodie::canBePickedUpByTunnelMan() const {
    return true;
}
int Goodie::getPoints() const { return points; }


//==================================================================================================
// BarrelOfOil Implementations
//==================================================================================================
BarrelOfOil::BarrelOfOil(StudentWorld* world_ptr, int startX, int startY)
    : Goodie(TID_BARREL, startX, startY, world_ptr, 1000, 0, false) {}

BarrelOfOil::~BarrelOfOil() {}

void BarrelOfOil::activate(TunnelMan* tunnelman) {
    getWorld()->playSound(SOUND_FOUND_OIL);
    getWorld()->decrementBarrelsRemaining();
}

//==================================================================================================
// Gold Implementations
//==================================================================================================

Gold::Gold(StudentWorld* world_ptr, int startX, int startY)
    : Goodie(TID_GOLD, startX, startY, world_ptr, 10, 0, false),
      goldState(State::PERMANENT_FOR_TUNNELMAN), totalTicks(0), pickedUpByProtester(false) {}

Gold::Gold(StudentWorld* world_ptr, int startX, int startY, bool isTemporary)
    : Goodie(TID_GOLD, startX, startY, world_ptr, 0, 0, true),
      goldState(State::TEMPORARY_FOR_PROTESTER), totalTicks(100), pickedUpByProtester(false) {}

Gold::~Gold() {}

void Gold::doSomething() {
    if (!isAlive()) return;

    if (goldState == State::PERMANENT_FOR_TUNNELMAN) {
        Goodie::doSomething();
    } else {
        totalTicks--;
        if (totalTicks <= 0) {
            setDead();
            return;
        }
        if (getWorld()->checkAndHandleProtesterGoldPickup(this, getX(), getY())) {
            setDead();
            return;
        }
    }
}

bool Gold::canBePickedUpByProtester() const {
    return goldState == State::TEMPORARY_FOR_PROTESTER; // Use enum class
}

void Gold::setPickedUpByProtester(bool pickedUp){
     pickedUpByProtester = pickedUp;
}
bool Gold::wasPickedUpByProtester() const {
    return pickedUpByProtester;
}


void Gold::activate(TunnelMan* tunnelman) {
    if (goldState == State::PERMANENT_FOR_TUNNELMAN) { // Use enum class
        getWorld()->playSound(SOUND_GOT_GOODIE);
        tunnelman->addGold();
    }
}

//==================================================================================================
// TemporaryGoodie Implementations
//==================================================================================================
TemporaryGoodie::TemporaryGoodie(int imageID, int startX, int startY, StudentWorld* world_ptr, int point_value, int lifetime)
    : Goodie(imageID, startX, startY, world_ptr, point_value, 0, true), totalTicksRemaining(lifetime) {}

TemporaryGoodie::~TemporaryGoodie() {}

void TemporaryGoodie::doSomething() {
    if (!isAlive()) return;

    totalTicksRemaining--;
    if (totalTicksRemaining <= 0) {
        setDead();
        return;
    }
    Goodie::doSomething();
}

//==================================================================================================
// SonarKit Implementations
//==================================================================================================
SonarKit::SonarKit(StudentWorld* world_ptr, int startX, int startY, int lifetime)
    : TemporaryGoodie(TID_SONAR, startX, startY, world_ptr, 75, lifetime) {}

SonarKit::~SonarKit() {}

void SonarKit::activate(TunnelMan* tunnelman) {
    getWorld()->playSound(SOUND_GOT_GOODIE);
    tunnelman->addSonar(2);
}

//==================================================================================================
// WaterPool Implementations
//==================================================================================================
WaterPool::WaterPool(StudentWorld* world_ptr, int startX, int startY, int lifetime)
    : TemporaryGoodie(TID_WATER_POOL, startX, startY, world_ptr, 100, lifetime) {}

WaterPool::~WaterPool() {}

void WaterPool::activate(TunnelMan* tunnelman) {
    getWorld()->playSound(SOUND_GOT_GOODIE);
    tunnelman->addWater(5);
}


//==================================================================================================
// Protester Base Class Implementations
//==================================================================================================
Protester::Protester(int imageID, StudentWorld* world_ptr, int initialHP)
    : Actor(imageID, 60, 60, left, 1.0, 0, world_ptr, initialHP, true),
      restingTicks(0),
      mustLeave(false),
      ticksSinceLastShout(16),
      ticksSinceLastPerpendicularTurn(201)
{
    int current_level_number= getWorld()->getLevel();
    ticksToWaitBetweenMoves = max(0, 3 - current_level_number / 4);
    pickNewRandomDirectionAndSteps();
}

Protester::~Protester() {}

void Protester::doSomething() {
    if (!isAlive()) return;

    if (isResting()) {
        decrementRestingTicks();
        return;
    }

    setRestingTicks(ticksToWaitBetweenMoves);
    ticksSinceLastShout++;
    ticksSinceLastPerpendicularTurn++;

    if (mustLeave) {
        if (getX() == 60 && getY() == 60) {
            setDead();
            return;
        }
        Direction exitDir = getNextMoveToExit();
        if (exitDir != none) {
             setDirection(exitDir);
             moveTo(getX() + (exitDir == right ? 1 : (exitDir == left ? -1 : 0)),
                    getY() + (exitDir == up ? 1 : (exitDir == down ? -1 : 0)));
        }
        return;
    }

    if (attemptToShout()) {
        return;
    }

    if (numSquaresToMoveInCurrentDirection <= 0) {
        pickNewRandomDirectionAndSteps();
    }

    Direction currentDir = getDirection();
    int nextX = getX(), nextY = getY();
    switch(currentDir) {
        case left: nextX--; break;
        case right: nextX++; break;
        case up: nextY++; break;
        case down: nextY--; break;
        default: break;
    }

    if (getWorld()->canProtesterMoveTo(this, nextX, nextY)) {
        moveTo(nextX, nextY);
        numSquaresToMoveInCurrentDirection--;
    } else {
        numSquaresToMoveInCurrentDirection = 0;
    }
}

bool Protester::annoy(int damagePoints) {
    if (mustLeave) return false;

    if (Actor::annoy(damagePoints)) {
        setMustLeaveOilField();
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
        return true;
    } else {
        getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
        int current_level_number= getWorld()->getLevel();
        int stunTicks = max(50, 100 - current_level_number * 10);
        setRestingTicks(stunTicks);
        return false;
    }
}

bool Protester::blocksMovement() const { return true; }
bool Protester::canBeHit() const { return !mustLeave; }
bool Protester::canBeBonked() const { return !mustLeave; }
bool Protester::canPickUpGold() const { return !mustLeave; }
bool Protester::isDamageable() const { return !mustLeave; }


void Protester::setMustLeaveOilField() {
    mustLeave = true;
    restingTicks = 0;
}

bool Protester::mustLeaveOilField() const {
    return mustLeave;
}

bool Protester::isResting() const {
    return restingTicks > 0;
}

void Protester::decrementRestingTicks() {
    if (restingTicks > 0) restingTicks--;
}

void Protester::setRestingTicks(int ticks_val) {
    restingTicks = ticks_val;
}

bool Protester::attemptToShout() {
    TunnelMan* tm = getWorld()->getTunnelMan();
    if (tm && tm->isAlive() && getWorld()->distance(getX(), getY(), tm->getX(), tm->getY()) <= 4.0) {
        bool facingPlayer = false;
        if (getDirection() == right && tm->getX() >= getX() && abs(tm->getY() - getY()) < SPRITE_HEIGHT) facingPlayer = true;
        else if (getDirection() == left && tm->getX() <= getX() && abs(tm->getY() - getY()) < SPRITE_HEIGHT) facingPlayer = true;
        else if (getDirection() == up && tm->getY() >= getY() && abs(tm->getX() - getX()) < SPRITE_WIDTH) facingPlayer = true;
        else if (getDirection() == down && tm->getY() <= getY() && abs(tm->getX() - getX()) < SPRITE_WIDTH) facingPlayer = true;

        if (facingPlayer && ticksSinceLastShout > 15) {
            getWorld()->playSound(SOUND_PROTESTER_YELL);
            tm->annoy(2);
            ticksSinceLastShout = 0;
            return true;
        }
    }
    return false;
}

bool Protester::lineOfSightToTunnelman(int& dx, int& dy, int& distance) {
    return getWorld()->hasClearPathToTunnelMan(this, getX(), getY(), getDirection(), dx, dy, distance);
}

bool Protester::moveTowards(int targetX, int targetY){
    Direction dir = getWorld()->getPathToCoordinate(getX(), getY(), targetX, targetY);
    if (dir != none && canMoveInDirection(dir)) {
        setDirection(dir);
        moveTo(getX() + (dir == right ? 1 : (dir == left ? -1 : 0)),
               getY() + (dir == up ? 1 : (dir == down ? -1 : 0)));
        return true;
    }
    return false;
}

void Protester::pickNewRandomDirectionAndSteps() {
    vector<Direction> possibleDirs;
    if (canMoveInDirection(up)) possibleDirs.push_back(up);
    if (canMoveInDirection(down)) possibleDirs.push_back(down);
    if (canMoveInDirection(left)) possibleDirs.push_back(left);
    if (canMoveInDirection(right)) possibleDirs.push_back(right);

    if (!possibleDirs.empty()) {
        setDirection(possibleDirs[rand() % possibleDirs.size()]);
    } else {
        Direction dirs[] = {up, down, left, right};
        setDirection(dirs[rand() % 4]);
    }
    numSquaresToMoveInCurrentDirection = (rand() % 53) + 8;
}

bool Protester::canMoveInDirection(Direction dir) const {
    int nextX = getX(), nextY = getY();
     switch(dir) {
        case left: nextX--; break;
        case right: nextX++; break;
        case up: nextY++; break;
        case down: nextY--; break;
        default: return false;
    }
    return getWorld()->canProtesterMoveTo(this, nextX, nextY);
}

GraphObject::Direction Protester::getNextMoveToExit() {
    return getWorld()->getPathToExit(getX(), getY());
}
GraphObject::Direction Protester::getNextMoveToTunnelMan(int targetX, int targetY){
    return getWorld()->getPathToCoordinate(getX(),getY(), targetX, targetY);
}


//==================================================================================================
// RegularProtester Implementations
//==================================================================================================
RegularProtester::RegularProtester(StudentWorld* world_ptr, int initialHP)
    : Protester(TID_PROTESTER, world_ptr, initialHP == 0 ? 5 : initialHP) {}

RegularProtester::~RegularProtester() {}

void RegularProtester::doSomething() {
    if (!isAlive()) return;
    if (isResting()) {
        decrementRestingTicks();
        return;
    }
    setRestingTicks(ticksToWaitBetweenMoves);
    ticksSinceLastShout++;
    ticksSinceLastPerpendicularTurn++;

    if (mustLeaveOilField()) {
        Protester::doSomething();
        return;
    }

    if (attemptToShout()) return;

    TunnelMan* tm = getWorld()->getTunnelMan();
    if (tm && tm->isAlive()) {
        int dx_to_tm, dy_to_tm, dist_to_tm;
        if (getWorld()->hasClearPathToTunnelMan(this, getX(), getY(), getDirection(), dx_to_tm, dy_to_tm, dist_to_tm)) {
            if (dist_to_tm > 4) {
                Direction targetDir = none;
                if (getX() == tm->getX()) {
                    targetDir = (getY() < tm->getY() ? up : down);
                } else if (getY() == tm->getY()) {
                    targetDir = (getX() < tm->getX() ? right : left);
                }

                if (targetDir != none && canMoveInDirection(targetDir)) {
                    setDirection(targetDir);
                    moveTo(getX() + (targetDir == right ? 1 : (targetDir == left ? -1 : 0)),
                           getY() + (targetDir == up ? 1 : (targetDir == down ? -1 : 0)));
                    numSquaresToMoveInCurrentDirection = 0;
                    return;
                }
            }
        }
    }

    numSquaresToMoveInCurrentDirection--;
    if (numSquaresToMoveInCurrentDirection <= 0) {
        pickNewRandomDirectionAndSteps();
    } else {
        if (ticksSinceLastPerpendicularTurn >= 200) {
            Direction currentDir = getDirection();
            vector<Direction> perpendicularOptions;
            if (currentDir == left || currentDir == right) {
                if (canMoveInDirection(up)) perpendicularOptions.push_back(up);
                if (canMoveInDirection(down)) perpendicularOptions.push_back(down);
            } else {
                if (canMoveInDirection(left)) perpendicularOptions.push_back(left);
                if (canMoveInDirection(right)) perpendicularOptions.push_back(right);
            }

            if (!perpendicularOptions.empty()) {
                setDirection(perpendicularOptions[rand() % perpendicularOptions.size()]);
                numSquaresToMoveInCurrentDirection = (rand() % 53) + 8;
                ticksSinceLastPerpendicularTurn = 0;
            }
        }
    }

    if (canMoveInDirection(getDirection())) {
         moveTo(getX() + (getDirection() == right ? 1 : (getDirection() == left ? -1 : 0)),
                getY() + (getDirection() == up ? 1 : (getDirection() == down ? -1 : 0)));
    } else {
        numSquaresToMoveInCurrentDirection = 0;
    }
}

void RegularProtester::acceptGold() {
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(25);
    setMustLeaveOilField();
}

bool RegularProtester::annoy(int damagePoints) {
    bool prev_must_leave = mustLeaveOilField();
    bool gave_up_from_base = Protester::annoy(damagePoints);

    if (getHP() <= 0 && !prev_must_leave) {
         if(getWorld()->wasAnnoyanceSourceBoulder(this)){
            getWorld()->increaseScore(500);
         } else {
            getWorld()->increaseScore(100);
         }
         return true;
    }
    return gave_up_from_base;
}


//==================================================================================================
// HardcoreProtester Implementations
//==================================================================================================
HardcoreProtester::HardcoreProtester(StudentWorld* world_ptr, int initialHP)
    : Protester(TID_HARD_CORE_PROTESTER, world_ptr, initialHP == 0 ? 20 : initialHP), ticksToStareAtGold(0) {}

HardcoreProtester::~HardcoreProtester() {}

void HardcoreProtester::doSomething() {
    if (!isAlive()) return;

    if (ticksToStareAtGold > 0) {
        ticksToStareAtGold--;
        setRestingTicks(1);
        return;
    }

    if (isResting()) {
        decrementRestingTicks();
        return;
    }
    setRestingTicks(ticksToWaitBetweenMoves);
    ticksSinceLastShout++;
    ticksSinceLastPerpendicularTurn++;


    if (mustLeaveOilField()) {
        Protester::doSomething();
        return;
    }

    if (attemptToShout()) return;

    TunnelMan* tm = getWorld()->getTunnelMan();
    if (tm && tm->isAlive()) {
        int M = 16 + getWorld()->getLevel() * 2;
        if (getWorld()->getPathDistanceToCoordinate(getX(), getY(), tm->getX(), tm->getY()) <= M) {
            Direction dirToTM = getNextMoveToTunnelMan(tm->getX(), tm->getY());
            if (dirToTM != none && canMoveInDirection(dirToTM)) {
                setDirection(dirToTM);
                moveTo(getX() + (dirToTM == right ? 1 : (dirToTM == left ? -1 : 0)),
                       getY() + (dirToTM == up ? 1 : (dirToTM == down ? -1 : 0)));
                return;
            }
        }
    }

    if (tm && tm->isAlive()) {
        int dx_to_tm, dy_to_tm, dist_to_tm;
        if (getWorld()->hasClearPathToTunnelMan(this, getX(), getY(), getDirection(), dx_to_tm, dy_to_tm, dist_to_tm)) {
            if (dist_to_tm > 4) {
                Direction targetDir = none;
                if (getX() == tm->getX()) { targetDir = (getY() < tm->getY() ? up : down); }
                else if (getY() == tm->getY()) { targetDir = (getX() < tm->getX() ? right : left); }

                if (targetDir != none && canMoveInDirection(targetDir)) {
                    setDirection(targetDir);
                    moveTo(getX() + (targetDir == right ? 1 : (targetDir == left ? -1 : 0)),
                           getY() + (targetDir == up ? 1 : (targetDir == down ? -1 : 0)));
                    numSquaresToMoveInCurrentDirection = 0;
                    return;
                }
            }
        }
    }

    numSquaresToMoveInCurrentDirection--;
    if (numSquaresToMoveInCurrentDirection <= 0) {
        pickNewRandomDirectionAndSteps();
    } else {
         if (ticksSinceLastPerpendicularTurn >= 200) {
            Direction currentDir = getDirection();
            vector<Direction> perpendicularOptions;
            if (currentDir == left || currentDir == right) {
                if (canMoveInDirection(up)) perpendicularOptions.push_back(up);
                if (canMoveInDirection(down)) perpendicularOptions.push_back(down);
            } else {
                if (canMoveInDirection(left)) perpendicularOptions.push_back(left);
                if (canMoveInDirection(right)) perpendicularOptions.push_back(right);
            }
            if (!perpendicularOptions.empty()) {
                setDirection(perpendicularOptions[rand() % perpendicularOptions.size()]);
                numSquaresToMoveInCurrentDirection = (rand() % 53) + 8;
                ticksSinceLastPerpendicularTurn = 0;
            }
        }
    }

    if (canMoveInDirection(getDirection())) {
         moveTo(getX() + (getDirection() == right ? 1 : (getDirection() == left ? -1 : 0)),
                getY() + (getDirection() == up ? 1 : (getDirection() == down ? -1 : 0)));
    } else {
        numSquaresToMoveInCurrentDirection = 0;
    }
}

void HardcoreProtester::acceptGold() {
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(50);
    int current_level_number = getWorld()->getLevel();
    ticksToStareAtGold = max(50, 100 - current_level_number * 10);
    setRestingTicks(ticksToStareAtGold);
}

bool HardcoreProtester::annoy(int damagePoints) {
    bool prev_must_leave = mustLeaveOilField();
    bool gave_up_from_base = Protester::annoy(damagePoints);
    if (getHP() <= 0 && !prev_must_leave) {
         if(getWorld()->wasAnnoyanceSourceBoulder(this)){
            getWorld()->increaseScore(500);
         } else {
            getWorld()->increaseScore(250);
         }
         return true;
    }
    return gave_up_from_base;
}

