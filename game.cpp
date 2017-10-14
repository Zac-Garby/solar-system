//
// Created by Zac G on 08/10/2017.
//

#include <functional>
#include "config.h"
#include "game.h"

Game::Game() {
    sidebar = new PlanetInspector(nullptr);

    sun = sf::CircleShape(SUN_RADIUS, 50);
    sun.setPosition(SYSTEM_X, SYSTEM_Y);
    sun.setFillColor(sf::Color::Yellow);
    sun.setOrigin(SUN_RADIUS, SUN_RADIUS);

    int numberInhabited = 0;

    // Continuously generate new sets of planets until enough are inhabited.
    while (numberInhabited < MIN_REQ_INHABITED || numberInhabited > MAX_REQ_INHABITED) {
        planets.clear();

        float dist = SUN_RADIUS + 100;

        for (int i = 0; i < NUM_PLANETS; i++) {
            Planet planet(dist);
            planets.push_back(planet);

            dist += planet.getPixelRadius() * 2 + 50;
        }

        numberInhabited = 0;

        for (auto &planet : planets) {
            if (planet.resources.store[Population] > 0) numberInhabited++;
        }
    }

    dragLine = sf::RectangleShape(sf::Vector2f(DRAG_LINE_WIDTH, DRAG_LINE_WIDTH));
    dragLine.setOrigin(DRAG_LINE_WIDTH / 2, DRAG_LINE_WIDTH / 2);
    dragLine.setFillColor(sf::Color(DRAG_LINE_COLOUR));
}

Scene *Game::update(float dt) {
    for (auto &planet : planets) {
        planet.update(this, dt);
    }

    for (auto &ship : ships) {
        ship.update(dt);

        if (ship.delivered) {
            // Remove the ship if it's got to its destination

            int index = -1;

            for (int i = 0; i < ships.size(); i++) {
                if (ships[i].id == ship.id) {
                    index = i;
                    break;
                }
            }

            if (index >= 0) {
                ships.erase(ships.begin() + index);
            }

            // Update the sidebar
            if (auto pi = dynamic_cast<PlanetInspector*>(sidebar)) {
                sidebar = new PlanetInspector(pi->planet);
            }
        }
    }

    sidebar->update(dt);

    return nullptr;
}

void Game::render(sf::RenderWindow *win) {
    positionDragLine(win);

    win->draw(sun);

    for (auto &planet : planets) {
        planet.renderOrbit(win);
    }

    if (dragging) win->draw(dragLine);

    for (auto &ship : ships) {
        ship.render(win);
    }

    for (auto &planet : planets) {
        planet.render(win);
    }

    sidebar->render(win);
}

Scene *Game::handleEvent(sf::Event *event) {
    sf::Vector2f pos;

    switch (event->type) {
        case sf::Event::KeyReleased:
            switch (event->key.code) {
                case sf::Keyboard::Escape:
                    return new MainMenu();
                default:
                    return nullptr;
            }

        case sf::Event::MouseButtonPressed:
            pos = sf::Vector2f(event->mouseButton.x, event->mouseButton.y);

            switch (event->mouseButton.button) {
                case sf::Mouse::Left:
                    mouseDown(pos);

                default:
                    break;
            }

            break;

        case sf::Event::MouseButtonReleased:
            pos = sf::Vector2f(event->mouseButton.x, event->mouseButton.y);

            switch (event->mouseButton.button) {
                case sf::Mouse::Left:
                    mouseUp(pos);

                default:
                    break;
            }

            break;

        default:
            break;
    }

    sidebar->handleEvent(event);

    return nullptr;
}

void Game::mouseDown(sf::Vector2f pos) {
    dragging = false;

    if (pos.x > SIDEBAR_WIDTH) {
        selectPlanet(nullptr);

        bool found = false;
        for (auto &planet : planets) {
            if (planet.contains(pos)) {
                selectPlanet(&planet);
                found = true;

                break;
            }
        }

        if (found) dragging = true;
    }
}

void Game::mouseUp(sf::Vector2f pos) {
    dragging = false;

    if (selected == nullptr) return;

    for (auto &planet : planets) {

        // If the planet is moused over, isn't selected and is populated.
        if (planet.contains(pos) &&
                selected != &planet &&
                selected->resources.store[Population] > 0) {
            sidebar = new ShipDesigner(this, selected, &planet);

            break;
        }
    }
}

void Game::selectPlanet(Planet *planet) {
    sidebar = new PlanetInspector(planet);
    selected = planet;
}

void Game::removeSidebar() {
    sidebar = new PlanetInspector(nullptr);
}

void Game::positionDragLine(sf::Window *win) {
    if (selected == nullptr) return;

    sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(*win));
    sf::Vector2f selPos = selected->getPosition(SYSTEM_CENTER);
    sf::Vector2f diff = mouse - selPos;

    float length = sqrtf(diff.x * diff.x + diff.y * diff.y);
    float angle = atan2f(diff.x, diff.y);

    dragLine.setPosition(selPos);
    dragLine.setSize(sf::Vector2f(length, DRAG_LINE_WIDTH));
    dragLine.setRotation(-angle * (180 / PI) + 90);
}
