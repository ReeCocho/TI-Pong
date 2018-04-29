/*
 *--------------------------------------
 * Program Name: Test Program
 * Author: Connor J. Bramham
 * License: None
 * Description: Testing/learning program.
 *--------------------------------------
*/

/*
	NOTE1: You might notice that I don't just clear the screen
		   and redraw the player. This is because, clearing the
		   screen to black is REALLY slow and causes the other
		   actors to look weird. Instead, when the player or 
		   AI move a single unit, I draw a black horizontal
		   line over top to cover them up. For the ball, I 
		   draw a black version over top, and then draw the
		   new ball.
		   
	NOTE2: When reading through the collision code, you can see
		   that the player's paddle collider is actually
		   bit bigger than the AIs. This is because users tend
		   to 'think' they made contact with the ball, when in
		   reality they didn't. To keep them from getting all
		   pissy we just make their paddle bigger without them
		   knowing.
 */

/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Standard headers (recommended) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Other */
#include <keypadc.h>
#include <graphx.h>

/* Padel dimensions */
#define PADDLE_WIDTH 4
#define PADDLE_HEIGHT 48

/* AI x position because it's long and I hate typing it out */
#define AI_X_POS (LCD_WIDTH - 4 - PADDLE_WIDTH)

/* Color palette */
uint16_t pong_palette[2] = 
{
	0x0000,	// Black
	0xFFFF	// White
};	

/* Player position */
int16_t player_pos = 0;

/* AI position */
int16_t ai_pos = 0;

/** Ball position and velocity */
float ball_x = LCD_WIDTH / 2;
float ball_y = LCD_HEIGHT / 2;
float ball_x_vel = 0.0f;
float ball_y_vel = 0.0f;

/* Scores */
uint8_t player_score = 0;
uint8_t ai_score = 0;

/* Player movement */
void move_player();

/* AI movement */
void move_ai();

/* Ball movement */
void move_ball();

/* Ball padel collision */
void ball_collide();

/* Draw score */
void draw_score();



void main(void) 
{
	// Generate random seed
	srandom((uint32_t)rtc_Time());

    // Initialize the 8bpp graphics 
    gfx_Begin();
	gfx_SetDrawBuffer();

	// Set palette
	gfx_SetPalette(&pong_palette, 2 * sizeof(uint16_t), 0);

	// Set color for player drawing and text
	gfx_SetColor(1);
	gfx_SetTextFGColor(1);
	
	// Randomize the balls x and y velocity
	ball_x_vel = ((float)(random() % 2) - 0.5f) * 3.0f;
	ball_y_vel = (((float)(random() % 32) / 32.0f) - 0.5f) * 3.0f;
	
	while(1)
	{
	    // Update kb_Data
        kb_Scan();
	
		// Quit game
		if(kb_Data[6] & kb_Clear) break;
	
		// Clear screen
		gfx_ZeroScreen();
		
		// Game updates
		move_player();
		move_ball();
		move_ai();
		draw_score();
		
		// Swap back buffer
		gfx_SwapDraw();
	}

    // Usual cleanup 
    gfx_End();
}

void move_player()
{
	// Player movement
	if (kb_Data[7] & kb_Down && player_pos != LCD_HEIGHT - PADDLE_HEIGHT)
		player_pos += 2;
	
	if (kb_Data[7] & kb_Up && player_pos != 0)
		player_pos -= 2;
	
	// Draw player
	gfx_FillRectangle(4, player_pos, PADDLE_WIDTH, PADDLE_HEIGHT);
}

void move_ai()
{
	// AI movement (Tries to catch up with the ball's Y position)
	if(ball_y < ai_pos + (PADDLE_HEIGHT / 2) && ai_pos != 0)
		ai_pos -= 2;
	else if(ball_y > ai_pos + (PADDLE_HEIGHT / 2) && ai_pos != LCD_HEIGHT - PADDLE_HEIGHT)
		ai_pos += 2;
	
	// Draw ai
	gfx_FillRectangle(AI_X_POS, ai_pos, PADDLE_WIDTH, PADDLE_HEIGHT);
}

void move_ball()
{
	// Move ball
	ball_x += ball_x_vel;
	ball_y += ball_y_vel;
	
	// Bounce off top and bottom
	if(ball_y - 1.0f <= 0.0f || ball_y + 1.0f >= LCD_HEIGHT)
		ball_y_vel = -ball_y_vel;
	
	// Check for collision
	ball_collide();
	
	// Check for bounces off the back wall
	if(ball_x - 1.0f <= 0.0f || ball_x + 1.0f >= LCD_WIDTH)
	{
		// Increment score
		if(ball_x - 1.0f <= 0.0f) ++ai_score;
		else if(ball_x + 1.0f >= LCD_WIDTH) ++player_score;
	
		// Reset position
		ball_x = LCD_WIDTH / 2;
		ball_y = LCD_HEIGHT / 2;
		
		// Randomize the balls x and y velocity
		ball_x_vel = ((float)(random() % 2) - 0.5f) * 3.0f;
		ball_y_vel = (((float)(random() % 32) / 32.0f) - 0.5f) * 3.0f;
	}
	
	// Draw ball
	gfx_FillRectangle((int16_t)ball_x - 1, (int16_t)ball_y - 1, 3, 3);
}

void ball_collide()
{
	float pos_diff;

	// Player collision
	if
	(
		ball_x - 1.0f <= 4 + PADDLE_WIDTH && 		// MAX X
		ball_y >= player_pos - 4 && 				// MIN Y
		ball_y <= player_pos + PADDLE_HEIGHT + 4	// MAX Y
	)
	{
		ball_x_vel = -ball_x_vel + 0.2f;
		pos_diff = ball_y - (float)(player_pos + (PADDLE_HEIGHT / 2));
		ball_y_vel += pos_diff / 16.0f;
	}
	
	// AI collision
	if
	(
		ball_x + 1.0f >= LCD_WIDTH - 4 - PADDLE_WIDTH &&		// MIN X
		ball_y >= ai_pos && 								// MIN Y
		ball_y <= ai_pos + PADDLE_HEIGHT						// MAX Y
	)
	{
		ball_x_vel = -ball_x_vel - 0.2f;
		pos_diff = ball_y - (float)(ai_pos + (PADDLE_HEIGHT / 2));
		ball_y_vel += pos_diff / 16.0f;
	}
}

void draw_score()
{
	// Player score
	gfx_SetTextXY(16, 4);
	gfx_PrintUInt((unsigned int)player_score, 3);
	
	// AI score
	gfx_SetTextXY(LCD_WIDTH - 48, 4);
	gfx_PrintUInt((unsigned int)ai_score, 3);
}