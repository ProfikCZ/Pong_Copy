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

void vector_rotate(sf::Vector2f &v, float angle = -1) // vector to rotate, random or not (implicit -1 input else use explicit input)
{
    // using sf::Transform + array of posible angle values + random int generator
    // transform ball velocity vector
    sf::Transform rotation;

    std::vector<float> deg = {0.f, 10.f, 20.f, 30.f, 40.f, 50.f, 60.f};
    float chosen_degrees;

    if (angle != -1)
    {
        rotation.rotate(angle);
        v = rotation.transformPoint(v);

        return;
    }
    else
    {
        int random = rand() % 7; // one of the 7 predefined values
        chosen_degrees = deg.at(random);

        if (rand() % 2) // clockwise or counter clockwise rotation
            chosen_degrees *= -1;

        if (rand() % 2) // reverse direction or not
            chosen_degrees += 180.f;

        rotation.rotate(chosen_degrees);
        v = rotation.transformPoint(v);
        return;
    }
}

class Player
{
    sf::RectangleShape rectangle;
    sf::Vector2f player_velocity;
    std::vector<sf::FloatRect> bounds;

    sf::Vector2f player_origin;

    int points;

public:
    Player(sf::Vector2f pos, sf::Vector2f size, sf::Vector2f velocity, std::vector<sf::RectangleShape> edge)
    {
        points = 0;
        player_origin = pos;

        rectangle.setSize(size);
        rectangle.setPosition(pos);
        player_velocity = velocity;
        for (const auto &border : edge)
        {
            bounds.push_back(border.getGlobalBounds());
        }
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
    }

    void move_down(float dt)
    {
        sf::FloatRect next = rectangle.getGlobalBounds();
        next.top += player_velocity.y * dt;

        if (!next.intersects(bounds[BOT]))
            rectangle.move(0.f, player_velocity.y * dt);
    }

    sf::FloatRect get_hitbox() const
    {
        return rectangle.getGlobalBounds();
    }

    sf::Vector2f get_position() const
    {
        return rectangle.getPosition();
    }

    sf::Vector2f get_size() const
    {
        return rectangle.getSize();
    }

    void add_point()
    {
        points++;
    }

    void reset()
    {
        rectangle.setPosition(player_origin);
    }
};

class Ball
{
    sf::RectangleShape ball;
    sf::Vector2f ball_origin;

    sf::Vector2f ball_velocity;
    sf::Vector2f ball_base_velocity;

    sf::Vector2f p1_normal_vector, p2_normal_vector;

    std::vector<sf::FloatRect> bounds;

public:
    Ball(sf::Vector2f pos, sf::Vector2f ball_size, sf::Vector2f velocity, std::vector<sf::RectangleShape> edge)
    {
        ball.setSize(ball_size);
        ball.setPosition(pos);
        ball_origin = pos;
        ball_velocity = ball_base_velocity = velocity;
        p1_normal_vector = ball_base_velocity, p2_normal_vector = -ball_base_velocity;

        for (const auto &border : edge)
        {
            bounds.push_back(border.getGlobalBounds());
        }
    }
    void draw_ball(sf::RenderWindow &window)
    {
        window.draw(ball);
    }

    void reset()
    {
        ball.setPosition(ball_origin);
        ball_velocity = ball_base_velocity;
        vector_rotate(ball_velocity);
    }

    sf::Vector2f get_position() const
    {
        return ball.getPosition();
    }

    sf::Vector2f get_size() const
    {
        return ball.getSize();
    }

    float relative_position(Player &w)
    {
        sf::Vector2f ball_center = ball.getPosition();
        ball_center.y += ball.getSize().y / 2;

        sf::Vector2f wall_center = w.get_position();
        wall_center.y += w.get_size().y / 2;

        float center_distance = ball_center.y - wall_center.y;

        float coef = center_distance / (w.get_size().y / 2);

        if (w.get_position().x > window_width / 2)
            return -coef;
        else
            return coef;
    }

    int move_ball(float dt, Player &p1, Player &p2)
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
            float angle_ratio = relative_position(p1);

            p1_normal_vector *= 1.05f;
            p2_normal_vector *= 1.05f;

            ball_velocity = p1_normal_vector;
            vector_rotate(ball_velocity, 60.f * angle_ratio);

            ball.setPosition(sf::Vector2f(player_width, ball.getPosition().y));
        }
        else if (next.intersects(p2.get_hitbox()))
        {
            float angle_ratio = relative_position(p2);

            p1_normal_vector *= 1.05f;
            p2_normal_vector *= 1.05f;

            ball_velocity = p2_normal_vector;
            vector_rotate(ball_velocity, 60.f * angle_ratio);

            ball.setPosition(sf::Vector2f(window_width - 2 * player_width, ball.getPosition().y));
        }
        else if (next.intersects(bounds[LEFT]))
        {
            // Win for left player (p2)
            p1_normal_vector = ball_base_velocity;
            p2_normal_vector = -ball_base_velocity;

            return 2;
        }
        else if (next.intersects(bounds[RIGHT]))
        {
            // Win for left player (p1)
            p1_normal_vector = ball_base_velocity;
            p2_normal_vector = -ball_base_velocity;

            return 1;
        }
        ball.move(ball_velocity * dt);
        return 0;
    }
};

void game_state_handle(Ball &b, Player &p1, Player &p2, bool &game_running, int player_number)
{
    b.reset();
    game_running = false;
    if (player_number == 1)
        p1.add_point();
    else
        p2.add_point();

    p1.reset();
    p2.reset();
}

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
    right.setPosition(sf::Vector2f(window_width + 10, 0));

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

    sf::Vector2f ball_size(player_width, player_width), ball_velocity(window_width / 2.f, 0.f), ball_pos((window_width - player_width) / 2, (window_height - player_width) / 2);

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "PONG");

    std::vector<sf::RectangleShape> outer_edge;
    make_border(outer_edge);

    Player p1(player1_pos, player_size, player_velocity, outer_edge);
    Player p2(player2_pos, player_size, player_velocity, outer_edge);

    Ball ball(ball_pos, ball_size, ball_velocity, outer_edge);

    sf::Event event;

    sf::Clock clock;

    bool game_running = false;

    int ret = 0;

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (!game_running && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            clock.restart();
            ball.reset();
            game_running = true;
        }

        float dt = clock.restart().asSeconds();

        window.clear(sf::Color::Black);

        if (game_running)
        {
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

            if ((ret = ball.move_ball(dt, p1, p2)) != 0)
            {
                game_state_handle(ball, p1, p2, game_running, ret);
            }
        }

        ball.draw_ball(window);

        p1.draw_player(window);
        p2.draw_player(window);

        window.display();
    }

    return 0;
}