//
// Created by Zac G on 08/10/2017.
//

#include <functional>
#include "planet.h"

float randRange(float min, float max) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = max - min;
    float r = random * diff;

    return min + r;
}

sf::Color randomColour() {
    std::vector<sf::Color> colours{
            sf::Color( 50, 150, 232),
            sf::Color( 40, 170, 110),

            sf::Color(101,  58, 232),
            sf::Color( 90,  48, 200),

            sf::Color(219,  74, 224),
            sf::Color(190,  65, 200),

            sf::Color(224,  74,  81),
            sf::Color(210,  90,  81),

            sf::Color(224, 174,  74),
            sf::Color(210, 150,  50),

            sf::Color(179, 224,  74),
            sf::Color(160, 200,  50),

            sf::Color( 72, 181,  83),
            sf::Color( 50, 200,  78),
    };

    return colours[rand() % colours.size()];
}

Planet::Planet(float distance) {
    distanceFromSun = distance;

    // Population has 40% chance to be 0, otherwise it's random from MIN_POPULATION to MAX_POPULATION
    resources.store[ResourceType::Population] = rand() > RAND_MAX * 0.4 ? int(randRange(MIN_POPULATION, MAX_POPULATION)) : 0;

    // Radius is from MIN_RADIUS to MAX_RADIUS
    radius = randRange(MIN_RADIUS, MAX_RADIUS);

    // Mass is radius * a random float from 0.75 to 1.25
    mass = radius * randRange(0.75, 1.25);

    // Angle is from 0 to 360
    angle = randRange(0, 360);

    // Speed is from MIN_PLANET_SPEED to MAX_PLANET_SPEED
    speed = randRange(MIN_PLANET_SPEED, MAX_PLANET_SPEED);

    colour = randomColour();
    name = Planet::randomName();

    float pixRadius = getPixelRadius();

    shape = sf::CircleShape(pixRadius);
    shape.setFillColor(colour);
    shape.setOrigin(pixRadius, pixRadius);

    orbit = sf::CircleShape(distanceFromSun, 80);
    orbit.setFillColor(sf::Color::Transparent);
    orbit.setOutlineColor(sf::Color(ORBIT_COLOUR));
    orbit.setOutlineThickness(5);
    orbit.setPosition(SYSTEM_X, SYSTEM_Y);
    orbit.setOrigin(distanceFromSun, distanceFromSun);

    resources.store[Metal] = 7500;
}

float Planet::getPixelRadius() {
    return radius / 1000;
}

void Planet::render(sf::RenderWindow *win) {
    win->draw(shape);
}

void Planet::renderOrbit(sf::RenderWindow *win) {
    win->draw(orbit);
}

void Planet::update(Game* game, float dt) {
    float angleDiff = atanf(speed / distanceFromSun);
    angle += angleDiff * dt;

    shape.setPosition(getPosition(SYSTEM_CENTER));

    while (shipQueue.size() > 0) {
        Spaceship ship = shipQueue.back();
        shipQueue.pop_back();
        game->ships.push_back(ship);
    }
}

sf::Vector2f Planet::getPosition(sf::Vector2f center) {
    sf::Vector2f direction(
            cos(angle),
            sin(angle)
    );

    return direction * distanceFromSun + center;
}

bool Planet::contains(sf::Vector2f point) {
    sf::Vector2f diff = point - shape.getPosition();
    float distance = diff.x * diff.x + diff.y * diff.y;

    return distance < (getPixelRadius() * getPixelRadius());
}

// TODO: Improve inhabitant name generation, maybe using Markov chains.
std::string Planet::getInhabitantName() {
    if (resources.store[ResourceType::Population] == 0) return "N/A";

    std::string inhabitants = name;

    char last = name[name.size() - 1];

    if (last == 'a') {
        // Can only end in 'a' if it ends in 'ia'
        // Therefore, 'ns' is added instead of 'ians'

        inhabitants += "ns";
    } else {
        // Replace trailing 's' with 't'
        // e.g. Mars -> Martians
        if (last == 's') inhabitants[inhabitants.size() - 1] = 't';

        inhabitants += "ians";
    }

    return inhabitants;
}

// TODO: Improve name generation, maybe using Markov chains.
std::string Planet::randomName() {
    std::string name;
    int letters = rand() % 4 + 4;

    char vowels[]{'a', 'e', 'i', 'o', 'u'};

    // Some consonants have been removed, since they can make bad planet names.
    char consonants[]{'b', 'c', 'd', 'f', 'g', 'k', 'l', 'm', 'n', 'p', 'q', 'r', 's', 't', 'v', 'z'};

    // Randomly sometimes start with a consonant
    if (rand() < RAND_MAX / 2) name += consonants[rand() % sizeof(consonants)];

    // Fill the string with n letters, alternating vowels and consonants
    for (int i = 0; i < letters; i += 2) {

        // Append a vowel-consonant pair
        name += vowels[rand() % sizeof(vowels)];
        name += consonants[rand() % sizeof(consonants)];

        // Add a 'u' after any 'q' which isn't at the end of the name
        if (name[name.size() - 1] == 'q' && i + 2 < letters) {
            name += 'u';
            i++;
        }
    }

    // Add a -ia suffix a quarter of the time
    if (rand() < RAND_MAX / 4) name += "ia";

    // Capitalise first letter
    name[0] -= 32;

    return name;
}

void Planet::launchShip(Resources res, Planet *destination) {
    Spaceship ship(this, destination, res);
    shipQueue.push_back(ship);
}
