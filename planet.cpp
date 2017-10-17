//
// Created by Zac G on 08/10/2017.
//

#include "planet.h"

// Selects a random float in the range of min to max.
float randRange(float min, float max) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = max - min;
    float r = random * diff;

    return min + r;
}

// Selects a random colour out of a list of
// 14. They're all quite nice.
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

    earthLikeness = randRange(MIN_EARTH_LIKENESS, MAX_EARTH_LIKENESS);
    radius = earthLikeness * EARTH_RADIUS;
    mass = 4.0f / 3.0f * PI * (radius * radius * radius) * EARTH_DENSITY;

    // Gravity acceleration in surface is calculated as G*M/r^2.
    // Expressed relative to Earth Gravity acceleration in surface (g).
    gravity = GRAVITY_CONST * mass / (radius * radius) / EARTH_G;

    // Find surface area and then calculate capacity based on density per area.
    capacity = (4 * PI * radius * radius) * randRange(MIN_DENSITY, MAX_DENSITY);

    // biodiveristy is just the Earth-likeness of the planet scaled between 1-10
    // this assumes a bigger planet (earthLikeness >> 1) would be more diverse, etc.
    biodiversity = ((((earthLikeness - MIN_EARTH_LIKENESS) * (10-1)) / (MAX_EARTH_LIKENESS - MIN_EARTH_LIKENESS)) + 1);

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

    orbit = sf::CircleShape(distanceFromSun, 100);
    orbit.setFillColor(sf::Color::Transparent);
    orbit.setOutlineColor(sf::Color(ORBIT_COLOUR));
    orbit.setOutlineThickness(N(2));
    orbit.setPosition(SYSTEM_X, SYSTEM_Y);
    orbit.setOrigin(distanceFromSun, distanceFromSun);

    float borderPixRadius = getBorderPixelRadius();
    border = sf::CircleShape(borderPixRadius);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(colour);
    border.setOutlineThickness(PLANET_BORDER_THICKNESS);
    border.setOrigin(borderPixRadius, borderPixRadius);
}

float Planet::getPixelRadius() {
    return radius / (ASSUMED_WIDTH / WIDTH_RATIO) * RADIUS_TO_PIXEL_FACTOR;
}

float Planet::getBorderPixelRadius() {
    return getPixelRadius() + PLANET_BORDER_EXTRA_PIXELS + PLANET_BORDER_THICKNESS;
}

void Planet::render(sf::RenderWindow *win) {
    win->draw(shape);
}

void Planet::renderOrbit(sf::RenderWindow *win) {
    win->draw(orbit);
}

void Planet::renderBorder(sf::RenderWindow *win) {
    win->draw(border);
}

void Planet::update(Game* game, float dt) {
    float angleDiff = atanf(speed / distanceFromSun);
    angle += angleDiff * dt;

    sf::Vector2f newPosition = getPosition(SYSTEM_CENTER);
    shape.setPosition(newPosition);
    border.setPosition(newPosition);

    while (shipQueue.size() > 0) {
        Spaceship ship = shipQueue.back();
        shipQueue.pop_back();
        game->ships.push_back(ship);
    }

	// TODO have people die off if there's not enough food
    // Update Population using logistic model
    int initialPop = resources.store[Population]; // Initial population of planet
    double k = 0.1;                               // Relative growth rate coefficient
    double A = (capacity - initialPop) / initialPop;
    double e = std::exp(1.0);

    if (initialPop != 0) {
        resources.store[Population] = int(capacity / (1 + A * pow(e, (-k * dt))));
    }

	// TODO: would like to implement people management ex. # of framers, scientists, engineers, 
	// laborers and resource production would be based of this
	int number_of_farmers = resources.store[Population];
    if(number_of_farmers > farmersCap) {
        number_of_farmers = farmersCap;
    }

    // Update others resources stats

	// species growth is 1 in 100000 with an advantage given based off biodiversity
	int growth = 100001 / biodiversity;
	if (rand() % growth < 1) {
		resources.store[Species] += 1;
	}
    resources.store[Metal] += randRange(0, 10000);
    resources.store[Wood] += randRange(0, 10000);
    resources.store[Water] += randRange(0, 10000);

	// If there's no people their shouldn't be any weapons
	if (resources.store[Population] > 0) {
		resources.store[Weaponary] = randRange(0, 10000);
	}

	// Each farmer on the planet can grow food equal to the bio diversity of the planet, however each person must eats one
  if(!resources.store[Population]) { // No farmers, no food
      resources.store[Food] = 0;
  } else {
      resources.store[Food] = int((number_of_farmers*biodiversity)/std::sqrt(resources.store[Population]));
  }

    // Food cap
    if(resources.store[Food] > foodCap) {
        resources.store[Food] = foodCap;
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

bool Planet::isInhabited() {
    return resources.store[Population] > 0;
}
