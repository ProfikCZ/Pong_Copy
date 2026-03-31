#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>

enum
{
    TOP = 0,
    BOT = 1,
    LEFT = 2,
    RIGHT = 3
};

float window_width = 1920, window_height = 1080; // note: aspect ratio is 16:9 and variables not adjusting for a different one
float player_height = window_height * 0.15f, player_width = player_height / 8;

void vector_rotate(/*vector to rotate, random or not (implicit -1 input else use explicit input)*/)
{
    // using sf::Transform + array of posible angle values + random int generator
    // transform ball velocity vector
}

class Player
{
    sf::RectangleShape rectangle;
    sf::Vector2f player_velocity;
    std::vector<sf::FloatRect> bounds;
    sf::FloatRect player_hitbox;

public:
    Player(sf::Vector2f pos, sf::Vector2f size, sf::Vector2f velocity, std::vector<sf::RectangleShape> edge)
    {
        rectangle.setSize(size);
        rectangle.setPosition(pos);
        player_velocity = velocity;
        for (const auto &border : edge)
        {
            bounds.push_back(border.getGlobalBounds());
        }
        player_hitbox = rectangle.getGlobalBounds();
    }

    void draw_player(sf::RenderWindow &window)
    {
        window.draw(rectangle);
    }

    void move_up(float dt)
    {
        sf::FloatRect next = rectangle.getGlobalBounds();
        next.top -= player_velocity.y * dt;

        if (!next.intersects(bounds[TOP]))
            rectangle.move(0.f, -player_velocity.y * dt);
        player_hitbox = rectangle.getGlobalBounds();
    }

    void move_down(float dt)
    {
        sf::FloatRect next = rectangle.getGlobalBounds();
        next.top += player_velocity.y * dt;

        if (!next.intersects(bounds[BOT]))
            rectangle.move(0.f, player_velocity.y * dt);
        player_hitbox = rectangle.getGlobalBounds();
    }

    sf::FloatRect get_hitbox()
    {
        return player_hitbox;
    }
};

class Ball
{
    sf::RectangleShape ball;
    sf::Vector2f ball_velocity;
    std::vector<sf::FloatRect> bounds;

public:
    Ball(sf::Vector2f pos, sf::Vector2f ball_size, sf::Vector2f velocity, std::vector<sf::RectangleShape> edge)
    {
        ball.setSize(ball_size);
        ball.setPosition(pos);
        ball_velocity = velocity;
        for (const auto &border : edge)
        {
            bounds.push_back(border.getGlobalBounds());
        }
    }
    void move_ball(float dt, Player &p1, Player &p2)
    {
        sf::FloatRect next = ball.getGlobalBounds();
        next.top += ball_velocity.y * dt;
        next.left += ball_velocity.x * dt;

        if (next.intersects(bounds[TOP]) || next.intersects(bounds[BOT]))
        {
            ball_velocity.y *= -1;
            ball.move(ball_velocity * dt);
        }
        else if (next.intersects(p1.get_hitbox()))
        {
            ball_velocity.x *= -1;
            ball.setPosition(sf::Vector2f(player_width, ball.getPosition().y));
        }
        else if (next.intersects(p2.get_hitbox()))
        {
            ball_velocity.x *= -1;
            ball.setPosition(sf::Vector2f(window_width - 2 * player_width, ball.getPosition().y));
        }
        else if (next.intersects(bounds[LEFT]))
        {
            // Win for right player
            // ball.reset
        }
        else if (next.intersects(bounds[RIGHT]))
        {
            // Win for left player
            // ball.reset
        }
        ball.move(ball_velocity * dt);
    }

    void draw_ball(sf::RenderWindow &window)
    {
        window.draw(ball);
    }
};

void make_border(std::vector<sf::RectangleShape> &arr)
{
    sf::RectangleShape top;
    sf::RectangleShape bot;
    sf::RectangleShape left;
    sf::RectangleShape right;

    top.setSize(sf::Vector2f(window_width, 10));
    bot.setSize(sf::Vector2f(window_width, 10));
    left.setSize(sf::Vector2f(10, window_height));
    right.setSize(sf::Vector2f(10, window_height));

    top.setPosition(sf::Vector2f(0, -10));
    bot.setPosition(sf::Vector2f(0, window_height + 10));
    left.setPosition(sf::Vector2f(-10, 0));
    right.setPosition(sf::Vector2f(0, window_width + 10));

    arr.push_back(top);
    arr.push_back(bot);
    arr.push_back(left);
    arr.push_back(right);
}

int main()
{

    sf::Vector2f player_velocity(0, window_width / 2);

    sf::Vector2f player_size(player_width, player_height);
    sf::Vector2f player1_pos(0, (window_height - player_height) / 2);
    sf::Vector2f player2_pos(window_width - player_width, (window_height - player_height) / 2);

    sf::Vector2f ball_size(player_width, player_width), ball_velocity(window_width / 2, 0.f), ball_pos((window_width - player_width) / 2, (window_height - player_width) / 2);

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "PONG");

    std::vector<sf::RectangleShape> outer_edge;
    make_border(outer_edge);

    Player p1(player1_pos, player_size, player_velocity, outer_edge);
    Player p2(player2_pos, player_size, player_velocity, outer_edge);

    Ball ball(ball_pos, ball_size, ball_velocity, outer_edge);

    sf::Event event;

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            p1.move_up(dt);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            p1.move_down(dt);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            p2.move_up(dt);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            p2.move_down(dt);
        }

        ball.move_ball(dt, p1, p2);
        ball.draw_ball(window);

        p1.draw_player(window);
        p2.draw_player(window);

        window.display();
    }

    return 0;
}