#include "../include/antworld.h"

#include <vector>
#include <limits>
#include <cmath>


// Finds the visible food that is cheapest to collect
// while still leaving enough energy for the ant to return home.
Coord findBestFood(
    Ant &ant,
    const std::vector<Coord> &visibleFood,
    MapTemplate &terrainMap
) {
    Coord bestFood = {-1, -1};
    int bestTotalCost = std::numeric_limits<int>::max();

    for (const Coord &food : visibleFood) {

        // Find cheapest path from the ant to the food.
        std::vector<Coord> pathToFood =
            shortestPath(terrainMap, ant.position, food);

        if (pathToFood.empty()) {
            continue;
        }

        int costToFood =
            calculatePathCost(terrainMap, pathToFood);


        // Find cheapest path from the food back home.
        std::vector<Coord> pathHome =
            shortestPath(terrainMap, food, ant.homeCoord);

        if (pathHome.empty()) {
            continue;
        }

        int costHome =
            calculatePathCost(terrainMap, pathHome);

        int totalCost = costToFood + costHome;


        // Only select food that the ant can collect
        // AND still have enough energy to return home.
        if (totalCost <= ant.energy &&
            totalCost < bestTotalCost) {

            bestTotalCost = totalCost;
            bestFood = food;
        }
    }

    return bestFood;
}


// Gives each ant a different area of the map to explore.
Coord chooseExploreDestination(
    const Ant &ant,
    const MapTemplate &terrainMap,
    int antIndex,
    int step
) {
    const int rows =
        static_cast<int>(terrainMap.size());

    const int cols =
        static_cast<int>(terrainMap[0].size());


    // Eight exploration targets for the eight ants.
    std::vector<Coord> targets = {
        {0, 0},                         // top-left
        {0, cols - 1},                  // top-right
        {rows - 1, 0},                  // bottom-left
        {rows - 1, cols - 1},           // bottom-right
        {0, cols / 2},                  // top-middle
        {rows - 1, cols / 2},           // bottom-middle
        {rows / 2, 0},                  // left-middle
        {rows / 2, cols - 1}            // right-middle
    };


    // Give each ant a different target.
    int targetIndex =
        antIndex % static_cast<int>(targets.size());

    Coord target = targets[targetIndex];


    // Find the cheapest route toward its exploration target.
    std::vector<Coord> path =
        shortestPath(
            terrainMap,
            ant.position,
            target
        );


    // If the ant has already reached its target,
    // send it toward another region.
    if (path.size() <= 1) {

        targetIndex =
            (targetIndex + 1 + (step % 7))
            % static_cast<int>(targets.size());

        target = targets[targetIndex];

        path =
            shortestPath(
                terrainMap,
                ant.position,
                target
            );
    }


    // Only move one cell along the exploration path.
    if (path.size() >= 2) {
        return path[1];
    }

    return ant.position;
}


void AntWorld::forage() {

    static int stepNumber = 0;


    // Give every living ant an action each game step.
    for (std::size_t i = 0;
         i < this->ants.size();
         ++i) {

        Ant &ant = this->ants[i];


        // Ignore ants with no energy.
        if (ant.energy <= 0) {
            continue;
        }


        // ---------------------------------------------------
        // 1. RETURN HOME
        // ---------------------------------------------------

        // Once an ant has food, returning it home
        // becomes its highest priority.
        if (ant.carryingFood) {

            ant.returnHome(
                this->terrainMap,
                this->foodMap
            );

            continue;
        }


        // ---------------------------------------------------
        // 2. SEARCH
        // ---------------------------------------------------

        // Look for food within the ant's sensor radius.
        std::vector<Coord> visibleFood =
            ant.foodScan(this->foodMap);


        // ---------------------------------------------------
        // 3. GO TO FOOD
        // ---------------------------------------------------

        if (!visibleFood.empty()) {

            Coord bestFood =
                findBestFood(
                    ant,
                    visibleFood,
                    this->terrainMap
                );


            // Move toward food only if we found
            // an affordable round trip.
            if (bestFood != Coord(-1, -1)) {

                ant.move(
                    this->terrainMap,
                    bestFood,
                    this->foodMap
                );

                continue;
            }
        }


        // ---------------------------------------------------
        // 4. EXPLORE
        // ---------------------------------------------------

        // No useful food is currently visible,
        // so explore another region of the map.
        Coord exploreDestination =
            chooseExploreDestination(
                ant,
                this->terrainMap,
                static_cast<int>(i),
                stepNumber
            );


        if (exploreDestination != ant.position) {

            ant.move(
                this->terrainMap,
                exploreDestination,
                this->foodMap
            );
        }
    }


    // Used to change exploration targets over time.
    ++stepNumber;
}