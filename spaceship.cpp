//
// Created by Zac G on 10/10/2017.
//

#include <functional>
#include "spaceship.h"

unsigned int Spaceship::nextID = 0;

Spaceship::Spaceship(Planet *sender, Planet *destination, Resources resources):
        sender(sender),
        destination(destination),
        resources(resources) {

    sender->resources = sender->resources - resources;

    delivered = false;

    id = nextID++;

    shape = sf::CircleShape(SHIP_RADIUS, SHIP_SHAPE);
    shape.setFillColor(sf::Color(SHIP_COLOUR));
    shape.setOrigin(SHIP_RADIUS, SHIP_RADIUS);

    position = sender->getPosition(SYSTEM_CENTER);
    shape.setPosition(position);

    velocity.x = 0;
    velocity.y = 0;
}

// TODO: Maybe add slight acceleration towards the sun, to simulate gravity
void Spaceship::update(float dt) {
    // Calculate velocity to apply
    sf::Vector2f direction = getDirectionToDestination();
    direction.x *= SHIP_SPEED * dt;
    direction.y *= SHIP_SPEED * dt;

    // Apply new velocity, divide by drag
    velocity += direction;
    velocity.x *= SHIP_DRAG;
    velocity.y *= SHIP_DRAG;

    // Update position
    position += velocity;
    shape.setPosition(position);

    if (hasReachedDestination()) {
        delivered = true;

        // Move resources from sender to destination
        destination->resources = destination->resources + resources;
    }
}

void Spaceship::render(sf::RenderWindow *win) {
    win->draw(shape);
}

sf::Vector2f Spaceship::getDirectionToDestination() {
    sf::Vector2f dest = destination->getPosition(SYSTEM_CENTER);
    sf::Vector2f diff = dest - position;
    float length = sqrtf(diff.x * diff.x + diff.y * diff.y);

    return diff / length;
}

bool Spaceship::hasReachedDestination() {
    sf::Vector2f diff = destination->getPosition(SYSTEM_CENTER) - position;
    float distance = sqrtf(diff.x * diff.x + diff.y * diff.y);

    return distance < destination->getPixelRadius();
}
