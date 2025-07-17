#include "StudentWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <queue>
using namespace std;

StudentWorld::StudentWorld(std::string assetPath)
    : GameWorld(assetPath), m_tunnelman(nullptr), m_barrelsRemaining(0),
      m_ticksSinceLastProtesterAdded(0),
      m_targetNumberOfProtesters(0),
      m_currentNumberOfProtestersOnField(0),
      m_lastAnnoyanceSource(nullptr) {
    for (int i = 0; i < OIL_FIELD_WIDTH; ++i) {
        for (int j = 0; j < EARTH_FIELD_HEIGHT; ++j) {
            m_earth[i][j] = nullptr;
        }
    }
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

GameWorld* createStudentWorld(string assetDir)
{
    return new StudentWorld(assetDir);
}

int StudentWorld::init() {
    m_barrelsRemaining = 0;
    m_ticksSinceLastProtesterAdded = 200;
    m_currentNumberOfProtestersOnField = 0;
    m_lastAnnoyanceSource = nullptr;

    m_tunnelman = new TunnelMan(this);

    for (int x = 0; x < OIL_FIELD_WIDTH; ++x) {
        for (int y = 0; y < EARTH_FIELD_HEIGHT; ++y) {
            if (x >= TUNNEL_SHAFT_X_START && x <= TUNNEL_SHAFT_X_END && y >= TUNNEL_SHAFT_Y_BOTTOM_NO_EARTH) {
                m_earth[x][y] = nullptr;
            } else {
                m_earth[x][y] = new Earth(this, x, y);
            }
        }
    }

    populateOilFieldWithObjects();

    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::populateOilFieldWithObjects() {
    int currentLevel = getLevel();


    int numBoulders = std::min(currentLevel / 2 + 2, 9);
    for (int i = 0; i < numBoulders; ++i) {
        int x, y;
        bool placed;
        do {
            placed = true;
            x = rand() % (OIL_FIELD_WIDTH - SPRITE_WIDTH + 1);
            y = (rand() % (56 - 20 + 1)) + 20;

            if (x + SPRITE_WIDTH -1 >= TUNNEL_SHAFT_X_START && x <= TUNNEL_SHAFT_X_END) {
                 placed = false; continue;
            }
            
            if (isAnyObjectNearby(x, y, 6.0, false)) {
                placed = false; continue;
            }

        } while (!placed);
        addActor(new Boulder(this, x, y));
    }

    int numGold = std::max(5 - currentLevel / 2, 2);
    for (int i = 0; i < numGold; ++i) {
        int x, y;
        bool placed;
        do {
            placed = true;
            x = rand() % (OIL_FIELD_WIDTH - SPRITE_WIDTH + 1);
            y = rand() % (EARTH_FIELD_HEIGHT - SPRITE_HEIGHT + 1);

            if (x + SPRITE_WIDTH -1 >= TUNNEL_SHAFT_X_START && x <= TUNNEL_SHAFT_X_END &&
                y + SPRITE_HEIGHT -1 >= TUNNEL_SHAFT_Y_BOTTOM_NO_EARTH && y <= TUNNEL_SHAFT_Y_TOP) {
                 placed = false; continue;
            }
            if (isAnyObjectNearby(x,y,6.0, false)) {placed = false; continue;}
        } while (!placed);
        addActor(new Gold(this, x, y));
    }

    int numBarrels = std::min(2 + currentLevel, 21);
    m_barrelsRemaining = numBarrels;
    for (int i = 0; i < numBarrels; ++i) {
        int x, y;
        bool placed;
        do {
            placed = true;
            x = rand() % (OIL_FIELD_WIDTH - SPRITE_WIDTH + 1);
            y = rand() % (EARTH_FIELD_HEIGHT - SPRITE_HEIGHT + 1);
            if (x + SPRITE_WIDTH -1 >= TUNNEL_SHAFT_X_START && x <= TUNNEL_SHAFT_X_END &&
                y + SPRITE_HEIGHT -1 >= TUNNEL_SHAFT_Y_BOTTOM_NO_EARTH && y <= TUNNEL_SHAFT_Y_TOP) {
                 placed = false; continue;
            }
            if (isAnyObjectNearby(x,y,6.0, false)) {placed = false; continue;}
        } while (!placed);
        addActor(new BarrelOfOil(this, x, y));
    }
}


int StudentWorld::move() {
    updateGameStatText();

    m_ticksSinceLastProtesterAdded++;

    if (m_tunnelman->isAlive()) {
        m_tunnelman->doSomething();
        if (!m_tunnelman->isAlive()) {
            decLives();
            playSound(SOUND_NONE);
            return GWSTATUS_PLAYER_DIED;
        }
        if (m_barrelsRemaining <= 0) {
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
    } else {
        decLives();
        playSound(SOUND_NONE);
        return GWSTATUS_PLAYER_DIED;
    }

    for (Actor* actor : m_actors) {
        if (actor->isAlive()) {
            actor->doSomething();
            if (!m_tunnelman->isAlive()) {
                decLives();
                playSound(SOUND_NONE);
                return GWSTATUS_PLAYER_DIED;
            }
            if (m_barrelsRemaining <= 0) {
                playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
    }

    removeDeadActors();

    addNewActorsDuringTick();

    if (!m_tunnelman->isAlive()){
         decLives();
         playSound(SOUND_NONE);
         return GWSTATUS_PLAYER_DIED;
    }

    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
    delete m_tunnelman;
    m_tunnelman = nullptr;

    for (int i = 0; i < OIL_FIELD_WIDTH; ++i) {
        for (int j = 0; j < EARTH_FIELD_HEIGHT; ++j) {
            delete m_earth[i][j];
            m_earth[i][j] = nullptr;
        }
    }

    for (Actor* actor : m_actors) {
        delete actor;
    }
    m_actors.clear();
}

bool StudentWorld::removeEarth(int x, int y) {
    if (x >= 0 && x < OIL_FIELD_WIDTH && y >= 0 && y < EARTH_FIELD_HEIGHT) {
        if (m_earth[x][y] != nullptr) {
            delete m_earth[x][y];
            m_earth[x][y] = nullptr;
            return true;
        }
    }
    return false;
}

bool StudentWorld::isEarthAt(int x, int y) const {
    if (x < 0 || x >= OIL_FIELD_WIDTH || y < 0 || y >= EARTH_FIELD_HEIGHT) {
        return false;
    }
    return m_earth[x][y] != nullptr;
}

bool StudentWorld::isEarthBelowBoulder(int x_boulder_left, int y_boulder_bottom) const {
    for (int i = 0; i < SPRITE_WIDTH; ++i) {
        if (isEarthAt(x_boulder_left + i, y_boulder_bottom - 1)) {
            return true;
        }
    }
    return false;
}

bool StudentWorld::isEarthUnderneath4x4(int x_topLeft, int y_topLeft) const {
    for (int i = 0; i < SPRITE_WIDTH; ++i) {
        for (int j = 0; j < SPRITE_HEIGHT; ++j) {
            if (isEarthAt(x_topLeft + i, y_topLeft + j)) {
                return true;
            }
        }
    }
    return false;
}


bool StudentWorld::isBoulderAtLocation(int x, int y, double checkRadius) const {
    for (Actor* actor : m_actors) {
        if (actor->isAlive() && dynamic_cast<Boulder*>(actor)) {
            if (x < actor->getX() + SPRITE_WIDTH && x + SPRITE_WIDTH > actor->getX() &&
                y < actor->getY() + SPRITE_HEIGHT && y + SPRITE_HEIGHT > actor->getY()) {
                return true;
            }
          
            if (checkRadius > 0.0) {
                 if (distance(x + SPRITE_WIDTH/2.0, y + SPRITE_HEIGHT/2.0,
                              actor->getX() + SPRITE_WIDTH/2.0, actor->getY() + SPRITE_HEIGHT/2.0) <= checkRadius) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool StudentWorld::isBoulderBlockingTunnelMan(int targetManX, int targetManY) const {
    for (Actor* actor : m_actors) {
        if (actor->isAlive() && dynamic_cast<Boulder*>(actor)) {
            double dist = distance(targetManX + SPRITE_WIDTH / 2.0, targetManY + SPRITE_HEIGHT / 2.0,
                                   actor->getX() + SPRITE_WIDTH / 2.0, actor->getY() + SPRITE_HEIGHT / 2.0);
            if (dist <= 3.0) {
                return true;
            }
        }
    }
    return false;
}


bool StudentWorld::canSquirtExistAt(int x, int y) const {
    if (x < 0 || x + SPRITE_WIDTH > OIL_FIELD_WIDTH || y < 0 || y + SPRITE_HEIGHT > GAME_BOARD_HEIGHT) {
        return false;
    }
    for (int i = 0; i < SPRITE_WIDTH; ++i) {
        for (int j = 0; j < SPRITE_HEIGHT; ++j) {
            if (isEarthAt(x + i, y + j)) return false;
        }
    }
    for (Actor* actor : m_actors) {
        if (actor->isAlive() && dynamic_cast<Boulder*>(actor)) {
            if (distance(x + SPRITE_WIDTH / 2.0, y + SPRITE_HEIGHT / 2.0,
                         actor->getX() + SPRITE_WIDTH / 2.0, actor->getY() + SPRITE_HEIGHT / 2.0) <= 3.0) {
                return false;
            }
        }
    }
    return true;
}


void StudentWorld::addActor(Actor* actor) {
    m_actors.push_back(actor);
}

void StudentWorld::revealNearbyObjects(int centerX, int centerY, double radius) {
    for (Actor* actor : m_actors) {
        if (dynamic_cast<BarrelOfOil*>(actor) ||
            (dynamic_cast<Gold*>(actor) && static_cast<Gold*>(actor)->getGoldState() == Gold::State::PERMANENT_FOR_TUNNELMAN) ) {
            if (!actor->isVisible() && distance(centerX, centerY, actor->getX(), actor->getY()) <= radius) {
                actor->setVisibleWithCheck(true);
            }
        }
    }
}

bool StudentWorld::annoyProtestersInRadius(Actor* instigator, int centerX, int centerY, double radius, int damage) {
    bool annoyedSomeone = false;
    m_lastAnnoyanceSource = instigator;
    for (Actor* actor : m_actors) {
        Protester* p = dynamic_cast<Protester*>(actor);
        if (p && p->isAlive() && p->canBeHit()) {
            if (distance(centerX, centerY, p->getX(), p->getY()) <= radius) {
                p->annoy(damage);
                annoyedSomeone = true;
            }
        }
    }
    return annoyedSomeone;
}

void StudentWorld::damageActorsInRadius(Actor* instigatorBoulder, int centerX, int centerY, double radius, int damage) {
    m_lastAnnoyanceSource = instigatorBoulder;

    if (m_tunnelman->isAlive() && m_tunnelman->canBeBonked()) {
        if (distance(centerX, centerY, m_tunnelman->getX(), m_tunnelman->getY()) <= radius) {
            m_tunnelman->annoy(damage);
        }
    }
    for (Actor* actor : m_actors) {
        Protester* p = dynamic_cast<Protester*>(actor);
        if (p && p->isAlive() && p->canBeBonked()) {
            if (distance(centerX, centerY, p->getX(), p->getY()) <= radius) {
                if(p->annoy(damage)){
                }
            }
        }
    }
}

bool StudentWorld::checkAndHandleProtesterGoldPickup(Gold* nugget, int goldX, int goldY) {
    for (Actor* actor : m_actors) {
        Protester* p = dynamic_cast<Protester*>(actor);
        if (p && p->isAlive() && p->canPickUpGold()) {
            if (distance(goldX, goldY, p->getX(), p->getY()) <= 3.0) {
                p->acceptGold();
                nugget->setPickedUpByProtester(true);
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::wasAnnoyanceSourceBoulder(Actor* annoyedActor) const {
    return dynamic_cast<Boulder*>(m_lastAnnoyanceSource) != nullptr;
}


void StudentWorld::decrementBarrelsRemaining() {
    if (m_barrelsRemaining > 0) {
        m_barrelsRemaining--;
    }
}


void StudentWorld::removeDeadActors() {
    for (auto it = m_actors.begin(); it != m_actors.end(); ) {
        if (!(*it)->isAlive()) {
            if (dynamic_cast<Protester*>(*it)) {
                m_currentNumberOfProtestersOnField--;
            }
            delete *it;
            it = m_actors.erase(it);
        } else {
            ++it;
        }
    }
}

void StudentWorld::addNewActorsDuringTick() {
    int currentLevel = getLevel();

    m_targetNumberOfProtesters = std::min(15, 2 + static_cast<int>(currentLevel * 1.5));
    int ticksToWaitProtester = std::max(25, 200 - currentLevel);

    if (m_ticksSinceLastProtesterAdded >= ticksToWaitProtester && m_currentNumberOfProtestersOnField < m_targetNumberOfProtesters) {
        int probabilityOfHardcore = std::min(90, currentLevel * 10 + 30);
        if ((rand() % 100) < probabilityOfHardcore) {
            addActor(new HardcoreProtester(this, 0));
        } else {
            addActor(new RegularProtester(this, 0));
        }
        m_currentNumberOfProtestersOnField++;
        m_ticksSinceLastProtesterAdded = 0;
    }

    int G_goodieChance = currentLevel * 25 + 300;
    if ((rand() % G_goodieChance) == 0) {
        int goodieLifetime = std::max(100, 300 - 10 * currentLevel);
        if ((rand() % 5) == 0) {
            addActor(new SonarKit(this, 0, 60, goodieLifetime));
        } else {
            int wx, wy;
            bool spotFound = false;
            for(int attempt = 0; attempt < 50; ++attempt) {
                wx = rand() % (OIL_FIELD_WIDTH - SPRITE_WIDTH + 1);
                wy = rand() % (EARTH_FIELD_HEIGHT - SPRITE_HEIGHT + 1);

                bool clearSpot = true;
                for(int r = 0; r < SPRITE_HEIGHT; ++r) {
                    for(int c = 0; c < SPRITE_WIDTH; ++c) {
                        if(isEarthAt(wx+c, wy+r)) {
                            clearSpot = false; break;
                        }
                    }
                    if(!clearSpot) break;
                }
                if(clearSpot && !isAnyObjectNearby(wx, wy, 0.0, false)) {
                    addActor(new WaterPool(this, wx, wy, goodieLifetime));
                    spotFound = true;
                    break;
                }
            }
        }
    }
}


void StudentWorld::updateGameStatText() {
    std::ostringstream oss;
    oss << "Lvl: " << std::setw(2) << std::right << getLevel()
        << "  Lives: " << std::setw(1) << getLives()
        << "  Hlth: " << std::setw(3) << std::right << (m_tunnelman->getHP() * 10) << "%"
        << "  Wtr: " << std::setw(2) << std::right << m_tunnelman->getWaterCount()
        << "  Gld: " << std::setw(2) << std::right << m_tunnelman->getGoldCount()
        << "  Oil Left: " << std::setw(2) << std::right << m_barrelsRemaining
        << "  Sonar: " << std::setw(2) << std::right << m_tunnelman->getSonarCount()
        << "  Scr: " << std::setw(6) << std::setfill('0') << std::right << getScore() << std::setfill(' ');
    setGameStatText(oss.str());
}

bool StudentWorld::isAnyObjectNearby(int x, int y, double radius, bool checkOnlyBoulders) const {
    if (m_tunnelman && m_tunnelman->isAlive()) {
         if (distance(x,y,m_tunnelman->getX(), m_tunnelman->getY()) <= radius) return true;
    }
    for (const auto* actor : m_actors) {
        if (!actor->isAlive()) continue;
        if (checkOnlyBoulders && !dynamic_cast<const Boulder*>(actor)) continue;

        if (distance(x,y,actor->getX(), actor->getY()) <= radius) {
            return true;
        }
    }
    return false;
}


double StudentWorld::distance(int x1, int y1, int x2, int y2) const {
    double c_x1 = x1 + SPRITE_WIDTH / 2.0;
    double c_y1 = y1 + SPRITE_HEIGHT / 2.0;
    double c_x2 = x2 + SPRITE_WIDTH / 2.0;
    double c_y2 = y2 + SPRITE_HEIGHT / 2.0;
    return std::sqrt(pow(c_x1 - c_x2, 2) + pow(c_y1 - c_y2, 2));
}

bool StudentWorld::canProtesterMoveTo(const Protester* protester, int targetX, int targetY) const {
    if (targetX < 0 || targetX + SPRITE_WIDTH > OIL_FIELD_WIDTH ||
        targetY < 0 || targetY + SPRITE_HEIGHT > GAME_BOARD_HEIGHT) {
        return false;
    }
    for (int i = 0; i < SPRITE_WIDTH; ++i) {
        for (int j = 0; j < SPRITE_HEIGHT; ++j) {
            if (isEarthAt(targetX + i, targetY + j)) return false;
        }
    }
    for (const auto* actor : m_actors) {
        if (actor->isAlive() && dynamic_cast<const Boulder*>(actor)) {
            if (distance(targetX, targetY, actor->getX(), actor->getY()) <= 3.0) {
                return false;
            }
        }
    }
    return true;
}

bool StudentWorld::hasClearPathToTunnelMan(const Protester* protester, int startX, int startY, Actor::Direction dir,
                                           int& dx_to_tm, int& dy_to_tm, int& path_dist_out) const {
    if (!m_tunnelman || !m_tunnelman->isAlive()) return false;

    int tmX_bl = m_tunnelman->getX();
    int tmY_bl = m_tunnelman->getY();

    dx_to_tm = tmX_bl - startX;
    dy_to_tm = tmY_bl - startY;
    path_dist_out = 0;

   
    if (startX == tmX_bl) {
        int step = (startY < tmY_bl) ? 1 : -1;
        int currentY = startY;
        while (currentY != tmY_bl) {
            currentY += step;
            path_dist_out++;
            if (path_dist_out > VIEW_HEIGHT) return false;
            for (int i = 0; i < SPRITE_WIDTH; ++i) {
                 if (isEarthAt(startX + i, currentY) || isBoulderAtLocation(startX + i, currentY, 0.0)) return false;
            }
        }
        return true;
    } else if (startY == tmY_bl) {
        int step = (startX < tmX_bl) ? 1 : -1;
        int currentX = startX;
        while (currentX != tmX_bl) {
            currentX += step;
            path_dist_out++;
            if (path_dist_out > VIEW_WIDTH) return false;
            for (int i = 0; i < SPRITE_HEIGHT; ++i) {
                if (isEarthAt(currentX, startY + i) || isBoulderAtLocation(currentX, startY + i, 0.0)) return false;
            }
        }
        return true;
    }
    return false;
}


struct PathfindingNode {
    int x, y, dist;
    Actor::Direction firstStepDir;
    vector<Actor::Direction> m_path;
};

Actor::Direction StudentWorld::getPathToCoordinate(int startX, int startY, int endX, int endY) {
    queue<PathfindingNode> q;
    vector<vector<bool>> visited(OIL_FIELD_WIDTH, vector<bool>(GAME_BOARD_HEIGHT, false));

    if(startX < 0 || startX >= OIL_FIELD_WIDTH || startY < 0 || startY >= GAME_BOARD_HEIGHT ||
       endX < 0 || endX >= OIL_FIELD_WIDTH || endY < 0 || endY >= GAME_BOARD_HEIGHT) return Actor::none;


    q.push({startX, startY, 0, Actor::none, {}});
    visited[startX][startY] = true;

    int dx_move[] = {0, 0, -1, 1};
    int dy_move[] = {1, -1, 0, 0};
    Actor::Direction dirs[] = {Actor::up, Actor::down, Actor::left, Actor::right};

    while (!q.empty()) {
        PathfindingNode curr = q.front();
        q.pop();

        if (curr.x == endX && curr.y == endY) {
            if (!curr.m_path.empty()) return curr.m_path[0];
            return Actor::none;
        }

        if (curr.dist > 200) continue;

        for (int i = 0; i < 4; ++i) {
            int nextX = curr.x + dx_move[i];
            int nextY = curr.y + dy_move[i];

            if (nextX >= 0 && nextX < OIL_FIELD_WIDTH && nextY >= 0 && nextY < GAME_BOARD_HEIGHT) {
                if (canProtesterMoveTo(nullptr, nextX, nextY) && !visited[nextX][nextY]) {
                    visited[nextX][nextY] = true;
                    std::vector<Actor::Direction> newPath = curr.m_path;
                    newPath.push_back(dirs[i]);
                    q.push({nextX, nextY, curr.dist + 1, (newPath.empty() ? Actor::none : newPath[0]), newPath});
                }
            }
        }
    }
    return Actor::none;
}

Actor::Direction StudentWorld::getPathToExit(int startX, int startY) {
    return getPathToCoordinate(startX, startY, 60, 60);
}

int StudentWorld::getPathDistanceToCoordinate(int startX, int startY, int endX, int endY){
    std::queue<PathfindingNode> q;
    std::vector<std::vector<int>> distGrid(OIL_FIELD_WIDTH, std::vector<int>(GAME_BOARD_HEIGHT, -1));

    if(startX < 0 || startX >= OIL_FIELD_WIDTH || startY < 0 || startY >= GAME_BOARD_HEIGHT ||
       endX < 0 || endX >= OIL_FIELD_WIDTH || endY < 0 || endY >= GAME_BOARD_HEIGHT) return 9999;


    q.push({startX, startY, 0, Actor::none, {}});
    distGrid[startX][startY] = 0;

    int dx_m[] = {0, 0, -1, 1};
    int dy_m[] = {1, -1, 0, 0};

    while (!q.empty()) {
        PathfindingNode curr = q.front();
        q.pop();

        if (curr.x == endX && curr.y == endY) {
            return curr.dist;
        }

        if(curr.dist > 200) continue;

        for (int i = 0; i < 4; ++i) {
            int nextX = curr.x + dx_m[i];
            int nextY = curr.y + dy_m[i];

            if (nextX >= 0 && nextX < OIL_FIELD_WIDTH && nextY >= 0 && nextY < GAME_BOARD_HEIGHT) {
                 if (canProtesterMoveTo(nullptr, nextX, nextY) && distGrid[nextX][nextY] == -1) {
                    distGrid[nextX][nextY] = curr.dist + 1;
                    q.push({nextX, nextY, curr.dist + 1, Actor::none, {}});
                }
            }
        }
    }
    return 9999;
}
