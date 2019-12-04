#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#endif

// Global var for screensize
int W, H;

// Global vars for game time
timer_id timer;
int second, minute = 0;
bool gamePause = false;

// Global vars for levels
bool gameOver = false;
int level = 1;
bool level_loaded = false;
int maxLevel = 1;
char activeChar = 'J';

// Struct for the characters
typedef struct charac char_spec;
struct charac
{
    double x;
    double y;
    double Dx;
    double Dy;
    double Sx;
    double Sy;
    int health;
    int charScore;
    int charLevelScore;
};

// Define some constants for the characters
#define TOM_IMG 'T'
#define TOM_MAX_HEALTH 5
#define JERRY_IMG 'J'
#define JERRY_MAX_HEALTH 5

// Globals for the characters
char_spec tom;
char_spec jerry;

// Setup the AI timer for Jerry and define the constant for a fireworks img
timer_id fireworkTimer;
#define FIREWORK_IMG '#'

//  Globals for the door and define constant for the door img
double doorX, doorY = 0.0;
bool doorCalc = false;
#define DOOR_IMG 'X'

// Struct for the walls
typedef struct env_wall wall_spec;
struct env_wall
{
    double x1;
    double y1;
    double x2;
    double y2;
};

// Define the constants for the walls
#define WALL_IMG '*'
#define BORDER_IMG '-'

// Globals for the walls
wall_spec *walls;
int wallCount = 0;

// Struct for the items
typedef struct items items_spec;
struct items
{
    double x;
    double y;

};

// Define the constants for the items
#define TRAP_IMG 'M'
#define TRAP_MAX 5
#define CHEESE_IMG 'C'
#define CHEESE_MAX 5

// Globals for the cheese
timer_id cheeseTimer;
items_spec cheeses[CHEESE_MAX];
int cheeseCount = 0;

// Globals for the traps
timer_id trapTimer;
items_spec traps[TRAP_MAX];
int trapCount = 0;

// Strcut for the fireworks
typedef struct moving_items weapon_spec;
struct moving_items
{
    double x;
    double y;
    double dx;
    double dy;
    bool draw;
};

// Globals for the fireworks
weapon_spec fireworks[50];
int fireworkCount = 0;

/*
 *  The function used to calculate the direction and initial position of the fireworks Jerry shoots
 */
void calculate_fireworks()
{
    float posOne = tom.x - jerry.x;
    float posTwo = tom.y - jerry.y;
    float divNum = sqrt((posOne*posOne) + (posTwo*posTwo));

    fireworks[fireworkCount].dx = posOne * 0.3/divNum;
    fireworks[fireworkCount].dy = posTwo * 0.3/divNum;

    fireworks[fireworkCount].x = jerry.x;
    fireworks[fireworkCount].y = jerry.y;
    fireworks[fireworkCount].draw = true;

    fireworkCount++;
}

/*
 *  Function to draw the door
 */
void draw_door()
{
    draw_char(round(doorX), round(doorY), DOOR_IMG);
} 

/*
 *  The function used to see if two xy coordinates are the same, a collision
 */
bool collided(double x0, double y0, double x1, double y1) {
    return round(x0) == round(x1) && round(y0) == round(y1);
}

/*
 *  The function used to see if a xy coordinate overlaps a wall, wall collisions
 */
bool wall_col(double x, double y)
{
    // Loop through all walls
    for (int i = 0; i < wallCount; i++)
    {   
        bool xColl = false;
        bool yColl = false;

        // Check for an x collision
        if (round(x) <= round(walls[i].x1) && round(x) >= round(walls[i].x2))
        {
            xColl = true;
        }

        // Check for an y collision
        if (round(y) <= round(walls[i].y1) && round(y) >= round(walls[i].y2))
        {
            yColl = true;
        }

        // If there is an x and y collision, there is a wall collision
        if (xColl && yColl)
        {
            return true;
        }
    }

    // Else just return false
    return false;
}

/*
 *  The function used to find a location to draw a door
 */
void calculate_door()
{
    // Set a local door coordinates to a random location within bounds
    double locdoorX = (rand() % (W-1 - 0 + 1)) + 0;
    double locdoorY = (rand() % (H-1 - 3 + 1)) + 3;

    // Check the door x and y are not overlapping a wall
    while (wall_col(locdoorX, locdoorY) || locdoorY < 3)
    {
        locdoorX = (rand() % (W-1 - 0 + 1)) + 0;
        locdoorY = (rand() % (H-1 - 3 + 1)) + 3;
    }

    // Set the global door coords to the local door coords
    doorX = locdoorX;
    doorY = locdoorY;
}

/*
 *  The function used to deal with everything related to the cheeese
 *  * Checking the timer to place a cheese at 2s if playing as Jerry
 *  * Calculating the random location of the Cheese
 *  * Drawing the cheese
 */
void draw_cheese()
{
    // Check if 2s is up and playing as Jerry
    if (timer_expired(cheeseTimer) && activeChar == 'J')
    {
        // Ensure not to have more than 5 cheese
        if (cheeseCount < CHEESE_MAX)
        {
            // Set a local cheese coordinates to a random location within bounds
            items_spec cheese;
            cheese.x = (rand() % (W-1 - 0 + 1)) + 0;
            cheese.y = (rand() % (H-1 - 3 + 1)) + 3;

            // Check the cheese coordinates are not overlapping a wall
            while (wall_col(cheese.x, cheese.y))
            {
                cheese.x = (rand() % (W-1 - 0 + 1)) + 0;
                cheese.y = (rand() % (H-1 - 3 + 1)) + 3;
            }

            // Set the cheese array as the local cheese
            cheeses[cheeseCount] = cheese;

            // Mark that another cheese is added
            cheeseCount++;
        }

        // Count down 2s again
        timer_reset(cheeseTimer);
    }

    // Loop through each cheese generated and draw it
    for (int i = 0; i < cheeseCount; i++)
    {
        draw_char(
            round(cheeses[i].x),
            round(cheeses[i].y),
            CHEESE_IMG
        );
    }
}

/*
 *  The function used to deal with everything related to the trap
 *  * Checking the timer to place a trap at 3s if playing as Jerry
 *  * Calculating the random location of the trap
 *  * Drawing the trap
 */
void draw_trap()
{
    // Check if 3s is up and playing as Jerry
    if (timer_expired(trapTimer) && activeChar == 'J')
    {
        // Ensure not to have more than 5 traps
        if (trapCount < TRAP_MAX)
        {
            // Set a local trap coordinates to a random location within bounds
            items_spec trap;
            trap.x = (rand() % (W-1 - 0 + 1)) + 0;
            trap.y = (rand() % (H-1 - 3 + 1)) + 3;

            // Check the trap coordinates are not overlapping a wall
            while (wall_col(trap.x, trap.y))
            {
                trap.x = (rand() % (W-1 - 0 + 1)) + 0;
                trap.y = (rand() % (H-1 - 3 + 1)) + 3;
            }

            // Set the trap array as the local trap
            traps[trapCount] = trap;

            // Mark another trap is added
            trapCount++;
        }
        // Count down 3s again
        timer_reset(trapTimer);
    }

    // Loop through each trap generated and draw it
    for (int i = 0; i < trapCount; i++)
    {
        draw_char(
            round(traps[i].x),
            round(traps[i].y),
            TRAP_IMG
        );
    }
}

/*
 *  The function used to reset everything between levels
 */
void reset_everything() {
    // Level based scoring reset
    jerry.charLevelScore = 0;
    tom.charLevelScore = 0;

    // Unpause the game before starting next level
    gamePause = false;

    // Reset the cheese and trap data
    cheeseCount = 0;
    trapCount = 0;
    for (int i = 0; i < CHEESE_MAX; i++)
    {
        cheeses[i].x = -100;
        cheeses[i].y = -100;
    }
    for (int i = 0; i < TRAP_MAX; i++)
    {
        traps[i].x = -100;
        traps[i].y = -100;
    }

    // Reset door data
    doorX = -100;
    doorY = -100;
    doorCalc = false;
}

/*
 *  The function used to signify the game is over
 */
void game_over() {
    
    // Prep and display the game over message
    clear_screen();

    const char *message[] = {
        "Game over!",
        "Press Q to quit or R to restart"
    };

    const int rows = 2;

    for (int i = 0; i < rows; i++) {
        int len = strlen(message[i]);
        int x = (W - len) / 2;
        int y = (H - rows) / 2 + i;
        draw_formatted(x, y, message[i]);
    }

    show_screen();

    // Next step after they pick an option
    int key = wait_char();
    if (key == 'q') // Tell main() game over
    {
        gameOver = true;
        return;
    }
    else if (key == 'r') // Reset everything to a fresh game
    {
        // Reset the level
        level = 1;
        level_loaded = false;
        
        // Reset the health
        tom.health = TOM_MAX_HEALTH;
        jerry.health = JERRY_MAX_HEALTH;
        
        // Reset the game score
        jerry.charScore = 0;
        tom.charScore = 0;

        // Reset the clock
        minute = 0;
        second = 0;

        // Run the level reset function
        reset_everything();
        return;
    }

}

/*
 *  The function used to calculate characters position on the level and its original AI
 */
void calculate_char(char type)
{
    // Set local variables for the calculation of AI
    double charX, charY = 0;
    double char_dir = rand() * M_PI * 2 / RAND_MAX;
    const double step = 0.1;

    // Check which character
    if (type == 'J')
    {
        // Set local coords based off level coords
        charX = jerry.Sx*(W-1);
        charY = jerry.Sy*(H-1);
    }
    else
    {
        // Set local coords based off level coords
        charX = tom.Sx*(W-1);
        charY = tom.Sy*(H-1);
    }

    // Make sure it doesnt spawn on status bar or below screen
    if (charY < 3)
    {
        charY = 3;
    }
    else if (charY > H-1)
    {   
        charY = H-1;
    }
    
    // Check which character
    if (type == 'J')
    {   
        // Set the character coords from the local coords
        jerry.x = charX;
        jerry.y = charY;

        // Set the AI movements
        jerry.Dx = step * cos(char_dir);
        jerry.Dy = step * sin(char_dir);
    }
    else
    {
        // Set the character coords from the local coords
        tom.x = charX;
        tom.y = charY;

        // Set the AI movements
        tom.Dx = step * cos(char_dir);
        tom.Dy = step * sin(char_dir);
    }
}

/*
 *  The function that controls Toms AI
 */
void move_tom(){
    // Set locale coords as the next step and calc the AI bounce
    int new_x = round(tom.x + tom.Dx);
    int new_y = round(tom.y + tom.Dy);
    double char_dir = rand() * M_PI * 2 / 180;
    const double step = 0.1;

    // Set bounce defaults
    bool xBounce = false;
    bool yBounce = false;

    // Check for x collision against border and walls
    if (new_x == 0 || new_x == screen_width() || wall_col(new_x, new_y)) {
        // Set new AI bounce for x and set x bounce to true
        tom.Dx = step * cos(char_dir);
        xBounce = true;
    }

    // Check for y collision against border and walls
    if (new_y == 3 || new_y == screen_height() || wall_col(new_x, new_y)) {
        // Set new AI bounce for y and set y bounce to true
        tom.Dy = step * sin(char_dir);
        yBounce = true;
    }

    // If x and y bounce are true set a slower AI bounce to standardise speed
    if (xBounce && yBounce)
    {
        tom.Dx = 0.05 * cos(char_dir);
        tom.Dy = 0.05 * sin(char_dir);
    }

    // Tom Jerry seeking if level is higher than 1
    if (level > 1)
    {
        // Set the seeking parameters
        float posOne = jerry.x - tom.x;
        float posTwo = jerry.y - tom.y;
        float divNum = sqrt((posOne*posOne) + (posTwo*posTwo));
        float dx = posOne * 0.3/divNum;
        float dy = posTwo * 0.3/divNum;
        float localtomx = tom.x;
        float localtomy = tom.y;
        bool wallcol = false;

        // Loop through expect path to check for walls
        while (round(localtomx) != round(jerry.x) && round(localtomy) != round(jerry.y))
        {
            // Emulate moving Tom
            localtomx += dx;
            localtomy += dy;

            // Check that he collides into a wall
            if (wall_col(localtomx, localtomy))
            {
                wallcol = true;
            }
        }

        // If there isn't a collision set the new auto movement to seeking
        if (!wallcol) {
            tom.Dx = dx;
            tom.Dy = dy;
        }
    }

    // If no collisions occured move tom forward
    if (!xBounce && !yBounce) {
        tom.x += tom.Dx;
        tom.y += tom.Dy;
    }
    
}

/*
 *  The function that controls Jerry AI
 */
void move_jerry(){
    // Set locale coords as the next step and calc the AI bounce
    int new_x = round(jerry.x + jerry.Dx);
    int new_y = round(jerry.y + jerry.Dy);
    double char_dir = rand() * M_PI * 2 / 180;
    const double step = 0.1;

    // Set bounce defaults
    bool xBounce = false;
    bool yBounce = false;

    // Check for x collision against border and walls
    if (new_x == 0 || new_x == screen_width() || wall_col(new_x, new_y)) {
        // Set new AI bounce for x and set x bounce to true
        jerry.Dx = step * cos(char_dir);
        xBounce = true;
    }

    // Check for y collision against border and walls
    if (new_y == 3 || new_y == screen_height() || wall_col(new_x, new_y)) {
        // Set new AI bounce for y and set y bounce to true
        jerry.Dy = step * sin(char_dir);
        yBounce = true;
    }

    // If x and y bounce are true set a slower AI bounce to standardise speed
    if (xBounce && yBounce)
    {
        jerry.Dx = 0.05 * cos(char_dir);
        jerry.Dy = 0.05 * sin(char_dir);
    }

    // Jerry cheese seeking if level is higher than 1
    if (level > 1)
    {
        // Loop through cheese
        for (int i = 0; i < cheeseCount; i++)
        {
            // Set the seeking parameters
            float posOne = cheeses[i].x - jerry.x;
            float posTwo = cheeses[i].y - jerry.y;
            float divNum = sqrt((posOne*posOne) + (posTwo*posTwo));
            float dx = posOne * 0.3/divNum;
            float dy = posTwo * 0.3/divNum;
            float localjerryx = jerry.x;
            float localjerryy = jerry.y;
            bool wallcol = false;

            // Loop through expect path to check for walls
            while (round(localjerryx) != round(cheeses[i].x) && round(localjerryy) != round(cheeses[i].y))
            {
                // Emulate moving Jerry
                localjerryx += dx;
                localjerryy += dy;

                // Check that he collides into a wall
                if (wall_col(localjerryx, localjerryy))
                {
                    wallcol = true;
                }
            }

            // If there isn't a collision set the new auto movement to seeking
            if (wallcol == false)
            {
                jerry.Dx = dx;
                jerry.Dy = dy;
                break;
            }
        }
    }

    // If no collisions occured move Jerry forward
    if (!xBounce && !yBounce) {
        jerry.x += jerry.Dx;
        jerry.y += jerry.Dy;
    }


}

/*
 *  The function that checks for a jerry collision into a trap 
 */
void trap_checks()
{
    if (activeChar == 'T')
    {
        // Loop through the traps
        for (int i = 0; i < trapCount; i++)
        {
            // Check for a collision with Jerry
            if (collided(round(jerry.x), round(jerry.y), round(traps[i].x), round(traps[i].y)))
            {
                // Increase Toms score and the level switch score then reset Jerry's position
                tom.charScore++;
                tom.charLevelScore++;
                calculate_char('J');

                // Shift every trap in the array to the left to overlap the trap that was hit (removes it and cleans array up)
                for(int j = i; j < trapCount-1; j++)
                {
                    traps[j] = traps[j+1];
                }

                // Signify a trap was removed
                trapCount--;
                break;
            }
        } 
    }
    else
    {
        // Loop through the traps
        for (int i = 0; i < trapCount; i++)
        {
            // Check for a collision with Jerry
            if (collided(round(jerry.x), round(jerry.y), round(traps[i].x), round(traps[i].y)))
            {
                // Lower Jerry's health and respawn him
                jerry.health--;
                calculate_char('J');

                // Shift every trap in the array to the left to overlap the trap (removes it and cleans array up)
                for(int j = i; j < trapCount-1; j++)
                {
                    traps[j] = traps[j+1];
                }

                // Signify a trap was hit
                trapCount--;
                break;
            }
        } 
    }
}


/*
 *  The function that checks for a Tom being hit by a firework as Tom
 */
void firework_tom()
{
    // Loop through all the fireworks
    for (int i = 0; i < fireworkCount; i++)
    {
        // Update firework location
        fireworks[i].x = fireworks[i].x + fireworks[i].dx;
        fireworks[i].y = fireworks[i].y + fireworks[i].dy;

        // If firework collides with a wall or the border
        if(wall_col(fireworks[i].x, fireworks[i].y) || fireworks[i].y < 3 || fireworks[i].y > H || fireworks[i].x < 0 || fireworks[i].x > W)
        {
            // Shift every firework in the array to the left to overlap the firework (removes it and cleans array up)
            for(int j = i; j < fireworkCount-1; j++)
            {
                fireworks[j] = fireworks[j+1];
            }

            // Signify a firework disappeared
            fireworkCount--;
            break;
        }

        // If firework collides with Tom
        if (collided(round(fireworks[i].x), round(fireworks[i].y), round(tom.x), round(tom.y)))
        {
            // Tom looses health and respawn Tom
            tom.health--;
            calculate_char('T');

            // Shift every firework in the array to the left to overlap the firework (removes it and cleans array up)
            for(int j = i; j < fireworkCount-1; j++)
            {
                fireworks[j] = fireworks[j+1];
            }

            // Signify a firework disappeared
            fireworkCount--;
            break;
        }
    }
}

/*
 *  The function that checks for a Tom being hit by a firework as Jerry
 */
void firework_jerry()
{
    // Loop through all the fireworks
    for (int i = 0; i < fireworkCount; i++)
    {
        // Update firework location
        fireworks[i].x = fireworks[i].x + fireworks[i].dx;
        fireworks[i].y = fireworks[i].y + fireworks[i].dy;

        // If firework collides with a wall or the border
        if(wall_col(fireworks[i].x, fireworks[i].y) || fireworks[i].y < 3 || fireworks[i].y > H || fireworks[i].x < 0 || fireworks[i].x > W)
        {
            // Shift every firework in the array to the left to overlap the firework (removes it and cleans array up)
            for(int j = i; j < fireworkCount-1; j++)
            {
                fireworks[j] = fireworks[j+1];
            }

            // Signify a firework disappeared
            fireworkCount--;
            break;
        }

        // If firework collides with Tom
        if (collided(round(fireworks[i].x), round(fireworks[i].y), round(tom.x), round(tom.y)))
        {
            // Add score for Jerry and respawn tom
            jerry.charScore++;
            calculate_char('T');

            // Shift every firework in the array to the left to overlap the firework (removes it and cleans array up)
            for(int j = i; j < fireworkCount-1; j++)
            {
                fireworks[j] = fireworks[j+1];
            }

            // Signify a firework disappeared
            fireworkCount--;
            break;
        }
    }
}

/*
 *  The function that checks for a character completing the level and going through the door
 */
void door_collision(char_spec character)
{
    // Check for character colliding with door
    if (collided(round(character.x), round(character.y), round(doorX), round(doorY)))
    {
        // If its not max level
        if (level != maxLevel)
        {
            // Go to the next level and reset all the level properties
            level++;
            level_loaded = false;
            reset_everything();
        }
        else
        {
            // Game Over 
            game_over();
        }
        
    }
}

/*
 *  The function that is responsible with how the environment affects Tom gameplay
 */
void update_envtom()
{

    // Loop through the cheese
    for (int i = 0; i < cheeseCount; i++)
    {
        // Check for a collision with Jerry
        if (collided(round(jerry.x), round(jerry.y), round(cheeses[i].x), round(cheeses[i].y)))
        {
            // Shift every cheese in the array to the left to overlap the cheese (removes it and cleans array up)
            for(int j = i; j < cheeseCount-1; j++)
            {
                cheeses[j] = cheeses[j+1];
            }

            // Signify a cheese was eaten
            cheeseCount--;
            break;
        }
    } 

    // Check if jerry collides into a trap
    trap_checks();

    // If the door hasnt been calculated yet and the user has hit score 5 for the level calculate the door
    if (!doorCalc && tom.charLevelScore >= 5)
    {
        calculate_door();
        doorCalc = true;
    }

    // If Tom runs into Jerry
    if (collided(round(tom.x), round(tom.y), round(jerry.x), round(jerry.y)))
    {    
        // Increase Toms score and the level switch score then reset Tom and Jerry's position
        tom.charScore += 5;
        tom.charLevelScore += 5;
        calculate_char('T');
        calculate_char('J'); 
    }

    // Check for Tom hit by firework
    firework_tom();

    // Check if Tom has gone through the door
    door_collision(tom);

    // If Toms health fully depletes game over
    if (tom.health == 0)
    {
        game_over();
    }

}

/*
 *  The function that is responsible with how the environment affects Tom gameplay
 */
void update_envjerry()
{
    // Loop through the cheese
    for (int i = 0; i < cheeseCount; i++)
    {
        // Check for a collision with Jerry
        if (collided(round(jerry.x), round(jerry.y), round(cheeses[i].x), round(cheeses[i].y)))
        {
            // Increase Jerry's score and the level switch score 
            jerry.charScore += 1;
            jerry.charLevelScore += 1;

            // Shift every cheese in the array to the left to overlap the cheese (removes it and cleans array up)
            for(int j = i; j < cheeseCount-1; j++)
            {
                cheeses[j] = cheeses[j+1];
            }

            // Signify a cheese was eaten
            cheeseCount--;
            break;
        }
    } 

    trap_checks();

    firework_jerry();

    // If Jerry runs into Tom
    if (collided(round(jerry.x), round(jerry.y), round(tom.x), round(tom.y)))
    {
        // Decrease Jerry's health and then reset Tom and Jerry's position
        jerry.health--;
        calculate_char('T');
        calculate_char('J');
    }

    // If the door hasnt been calculated yet and the user has hit score 5 for the level calculate the door
    if (!doorCalc && jerry.charLevelScore >= 5)
    {
        calculate_door();
        doorCalc = true;
    }

    // Check if Jerry has gone through the door
    door_collision(jerry);

    // If Jerry's health fully depletes game over
    if (jerry.health == 0)
    {
        game_over();
    }
}

/*
 *  The function that is responsible for user controlled Tom placing cheese
 */
void place_cheese()
{

    // Set a local cheese coordinates to Tom's coords
    items_spec cheese;
    cheese.x = tom.x;
    cheese.y = tom.y;

    // If cheese count is less than max amount of cheese
    if (cheeseCount < CHEESE_MAX)
    {
        // Set the cheese array as the local cheese
        cheeses[cheeseCount] = cheese;

        // Mark another cheese was added
        cheeseCount++;
    }
    else
    {   
        // Shift every cheese in the array to the left to overlap the cheese (removes it and cleans array up)
        for(int i = 0; i < cheeseCount-1; i++)
        {
            cheeses[i] = cheeses[i+1];
        }

        // Set the cheese array as the local cheese at the end
        cheeses[cheeseCount-1] = cheese;
    }
        
}

/*
 *  The function that is responsible for user controlled Tom placing traps
 */
void place_trap()
{
    // Set a local trap coordinates to Tom's coords
    items_spec trap;
    trap.x = tom.x;
    trap.y = tom.y;

    // If trap count is less than max amount of traps
    if (trapCount < TRAP_MAX)
    {
         // Set the trap array as the local trap
        traps[trapCount] = trap;

        // Mark another trap was added
        trapCount++;
    }
    else
    {
        // Shift every trap in the array to the left to overlap the trap (removes it and cleans array up)
        for(int i = 0; i < trapCount-1; i++)
        {
            traps[i] = traps[i+1];
        }

        // Set the trap array as the local trap at the end
        traps[trapCount-1] = trap;
    }    
}

/*
 *  The function that is responsible for user controlled Tom or delegating to AI Tom
 */
void update_tom(int ch) 
{

    // Check if the user is controlling Tom
    if (activeChar == 'T')
    {   
        // Moved character dependant on key press, bounds and wall collisions
        if (ch == 'a' && tom.x > 0 && scrape_char(tom.x-1, tom.y) != '*')  
        {
            tom.x--;
        }
        else if (ch == 'd' && tom.x < W - 1 && scrape_char(tom.x+1, tom.y) != '*') 
        {
            tom.x++;
        }
        else if (ch == 's' && tom.y < H - 1 && scrape_char(tom.x, tom.y+1) != '*') 
        {
            tom.y++;
        }
        else if (ch == 'w' && tom.y > 3 && scrape_char(tom.x, tom.x-1) != '*') 
        {
            tom.y--;
        }
        else if (ch == 'c') // Place cheese
        {
            place_cheese();
        }
        else if (ch == 'm') // Place trap
        {
            place_trap();
        }
    }
    else
    {
        // Ensure game is not paused and run AI Tom
        if (ch < 0 && !gamePause) 
        {
            move_tom();
        }
    }
    
}

/*
 *  The function that is responsible for user controlled Jerry or delegating to AI Jerry
 */
void update_jerry(int ch) 
{
    // Check if the user is controlling Jerry
    if (activeChar == 'J')
    {
        if (ch == 'a' && jerry.x > 0 && scrape_char(jerry.x-1, jerry.y) != '*') 
        {
            jerry.x--;
        }
        else if (ch == 'd' && jerry.x < W - 1 && scrape_char(jerry.x+1, jerry.y) != '*') 
        {
            jerry.x++;
        }
        else if (ch == 's' && jerry.y < H - 1 && scrape_char(jerry.x, jerry.y+1) != '*') 
        {
            jerry.y++;
        }
        else if (ch == 'w' && jerry.y > 3 && scrape_char(jerry.x, jerry.y-1) != '*') 
        {
            jerry.y--;
        }
        else if (ch == 'f' && level > 1) // Shoot firework
        {
            calculate_fireworks();
        }
    }
    else
    {
        // Ensure game is not paused and run AI Jerry
        if (ch < 0 && !gamePause) 
        {
            move_jerry();

            // AI Jerry shoots fireworks every 5 seconds
            if (timer_expired(fireworkTimer) && level > 1)
            {
                calculate_fireworks();
                timer_reset(fireworkTimer);
            }
        }
    }
}

/*
 *  The function that is responsible for calculating the walls and saving them
 */
void calculate_walls(double *wallArray)
{
    // Make a local wall with the inputs being from the file parse and set them within the bounds
    wall_spec wall;
    wall.x1 = round(wallArray[0]*(W-1));
    wall.y1 = round(wallArray[1]*(H- 1 - 3))+3;
    wall.x2 = round(wallArray[2]*(W-1));
    wall.y2 = round(wallArray[3]*(H- 1 - 3))+3;

    // Ensure that the walls are consitent so that wall collision doesn't do weird stuff
    if ((wall.x2)-(wall.x1) > 0)
    {
        wall.x1 = round(wallArray[2]*(W-1));
        wall.x2 = round(wallArray[0]*(W-1));
    }
    if ((wall.y2)-(wall.y1) > 0)
    {
        wall.y1 = round(wallArray[3]*(H- 1 - 3))+3;
        wall.y2 = round(wallArray[1]*(H- 1 - 3))+3;
    }

    // Add local wall to walls array and mark that there is another wall
    walls[wallCount] = wall;
    wallCount++;
}

/*
 *  The function that is responsible for parsing the file and calculating the character or walls
 */
void parse_file(FILE* file)
{
    // Loop through each line of the file until end of file
    while (!feof(file))
    {
        // Set some local variables
        char type;
        double var[4];

        // Scan the line into the local variables and count how many fields were scanned
        int count = fscanf(file, "%c %lf %lf %lf %lf\n", &type, &var[0], &var[1], &var[2], &var[3]);

        // Check if it is a character line
        if (count == 3)
        {
            // Set Tom variables if the character is Tom
            if (type == 'T')
            {
                tom.Sx = var[0];
                tom.Sy = var[1];
                calculate_char(type);
            }
            else // Set Jerry variables if its not Tom
            {
                jerry.Sx = var[0];
                jerry.Sy = var[1];
                calculate_char(type);
            }
            
        }
        else if (count == 5) // Check if its a wall line
        {
            if (type == 'W') // Confirm its a wall line and calculate the walls
            {
                calculate_walls(var);
            }
        }
    }
}

/*
 *  The function that is responsible for counting how many lines are in the file and allocating wall memory
 */
void count_file(FILE* file)
{
    // Count how many lines
    int lines = 0;
    while(!feof(file))
    {
        char ch = fgetc(file);
        if(ch == '\n')
        {
            lines++;
        }
    }

    // Allocate the wall array to have the amount of lines minus Tom and Jerry lines
    walls = malloc((lines-2) * sizeof(wall_spec));    
}

/*
 *  The function that is responsible for reading the input file then calling functions to deal with it
 */
void read_file(char *filepath)
{
    // Open the file
    FILE *openFile = fopen(filepath, "r");

    // Ensure the file opened
    if (openFile != NULL)
    {
        // Count how many lines and set the walls array then go back to start of file and parse it
        count_file(openFile);
        fseek(openFile, 0, SEEK_SET);
        parse_file(openFile);
        fclose(openFile);
    }
}

/*
 *  The function that is responsible for keeping track of the clock
 */
void timecheck()
{
    // Ensure thee game isn't paused
    if (!gamePause)
    {
        // Check that 1 second is up
        if (timer_expired(timer))
        {
            // Increment 1s
            second++;

            // Increment a minute in 1m
            if(second >= 60)
            {
                minute++;
                second = 0;
            }

            // Reset the timer
            timer_reset(timer);
        }
    }
}

/*
 *  The function that is responsible for drawing the walls from the walls array
 */
void draw_walls()
{
    // Loop through and draw
    for (int i = 0; i < wallCount; i++)
    {
        draw_line(
            walls[i].x1,
            walls[i].y1,
            walls[i].x2,
            walls[i].y2, 
            WALL_IMG
        );
    }
}

/*
 *  The function that is responsible for drawing the fireworks from the firework array
 */
void draw_fireworks()
{
    for (int i = 0; i < fireworkCount; i++)
    {
        if (fireworks[i].draw)
        {
            draw_char(fireworks[i].x, fireworks[i].y, FIREWORK_IMG);
        }
    }
}

/*
 *  The function that is responsible for drawing the single border between status bar and the game
 */
void draw_border()
{
    draw_line(0, 2, W, 2, BORDER_IMG);
}

/*
 *  The function that is responsible for drawing Tom
 */
void draw_tom()
{
    draw_char(tom.x, tom.y, TOM_IMG);
}

/*
 *  The function that is responsible for drawing Jerry
 */
void draw_jerry()
{
    draw_char(jerry.x, jerry.y, JERRY_IMG);
}

/*
 *  The function that is responsible for drawing the status bar
 */
void draw_statusbar()
{

    // Prime status bar 1
    char statusbartop[5][40] = {
        "Student Number: 10245065",
        "Score: ",
        "Lives: ",
        "Player: ",
        "Time: "
    };

    // Prime status bar 2
    char statusbarbottom[4][40] = {
        "Cheese: ",
        "Traps: ",
        "Fireworks: ",
        "Level: "
    };

    // Check if Jerry is being controlled
    if (activeChar == 'J')
    {
        // Set the status bar to Jerry specific content
        sprintf(statusbartop[1], "Score: %d", jerry.charScore);
        sprintf(statusbartop[2], "Lives: %d", jerry.health);
    }
    else
    {
        // Set the status bar to Tom specific content
        sprintf(statusbartop[1], "Score: %d", tom.charScore);
        sprintf(statusbartop[2], "Lives: %d", tom.health);
    }
    
    // Set the other status bar content
    sprintf(statusbartop[3], "Player: %c", activeChar);
    sprintf(statusbartop[4], "Time: %02d:%02d", minute, second);
    sprintf(statusbarbottom[0], "Cheese: %d", cheeseCount);
    sprintf(statusbarbottom[1], "Traps: %d", trapCount);
    sprintf(statusbarbottom[2], "Fireworks: %d", fireworkCount);
    sprintf(statusbarbottom[3], "Level: %d", level);

    // Marker to format the status bar horizontally
    int divisable = W/5;

    // Loop through each field in the status bar 1 and display it
    for (int i = 0; i < 5; i++)
    {
        draw_formatted(i*divisable, 0, statusbartop[i]);
    }

    // Loop through each field in the status bar 2 and display it
    for (int i = 0; i < 4; i++)
    {
        draw_formatted(i*divisable, 1, statusbarbottom[i]);
    }

}

/*
 *  The function that is responsible for drawing everything and deciding next level
 */
void draw_all(char *argv[])
{
    clear_screen();

    // Check if the level has been parsed and draw the walls
    if (level_loaded)
    {
        draw_walls();
    }
    else
    {
        // Clear array data
        for (int i = 0; i < wallCount; i++)
        {
            walls[i].x1 = 0;
            walls[i].y1 = 0;
            walls[i].x2 = 0;
            walls[i].y2 = 0;
        }

        // Parse the level and reset everything for the level to begin
        read_file(argv[level]);
        level_loaded = true;
        reset_everything();
    }

    // Draw the characters
    draw_jerry();
    draw_tom();

    // Draw the environment
    draw_cheese();
    draw_trap();
    draw_fireworks();

    // Draw the door when ready to change levels
    if (doorCalc)
    {
        draw_door();
    }

    // Draw the status bar
    draw_border();
    draw_statusbar();

    show_screen();
}

/*
 *  The function that is responsible for the main game loop
 */
void loop()
{

    // See what key the user types
    int key = get_char();
    if (key == 'q') { // Game over
        gameOver = true;
        return;
    }
    else if (key == 'i' && level <= maxLevel) // Progress to the next level
    {
        level++;
        level_loaded = false;
    }
    else if (key == 'i'  && level > maxLevel) // No more levels, game over
    {
        game_over();
    }
    else if (key == 'p') // Pause the game
    {
        gamePause = !gamePause;
    }
    else if (key == 'z' && level > 1) // Switch characters
    {
        if (activeChar == 'J')
        {
            activeChar = 'T';
        }
        else
        {
            activeChar = 'J';
        }
    }
    else // Let the game run normally
    {
        // Check if Jerry is being controlled
        if (activeChar == 'J') 
        {
            // Run Jerry specific environment
            update_envjerry();
        }
        else
        {
            // Run Tom specific environment
            update_envtom();
        }

        // Move character or AI can move them
        update_tom(key);
        update_jerry(key);

        // Keep the clock going
        timecheck();
    }
    
}

/*
 *  The function that is responsible for setting up the game
 */
void setup()
{
    // Create the various timers eessential for the game to work
    timer = create_timer(1000);
    cheeseTimer = create_timer(2000);
    trapTimer = create_timer(3000);
    fireworkTimer = create_timer(5000);

    // Set the characters health to max
    jerry.health = JERRY_MAX_HEALTH;
    tom.health = TOM_MAX_HEALTH;
}

/*
 *  The function that is the entry point
 */
int main(int argc, char *argv[]){
    // Get some screen stuff sorted
    setup_screen();
    W = screen_width();
    H = screen_height();

    // Set the max level dependant on how many args are parsed
    maxLevel = argc-2;

    // Setup the game
    setup();

    // Begin the game loop
    while (!gameOver) {
        draw_all(argv);
        loop();
        timer_pause(10);
    }

    return 0;
}