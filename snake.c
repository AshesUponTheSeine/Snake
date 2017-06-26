#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

/*
 * TODO:
 *  - Add high scores
 *  - Make placePellet more efficient?
 *  - Don't let users go backwards into themselves?
 */

#define SIZE 20
#define PELLET '.' | COLOR_PAIR(1)

struct pos { int x, y; };

// Returns the location of a valid pellet
// TODO: Keeps trying until it finds a vacant spot - not efficient late-game!
void placePellet(WINDOW* win) {
  struct pos pellet;

  do {
    pellet.x = rand()%SIZE + 1;
    pellet.y = rand()%SIZE + 1;
  } while(mvwinch(win, pellet.y, pellet.x) == '#');

  mvwaddch(win, pellet.y, pellet.x, PELLET);
}

// Moves the snake by 1 unit; updating both the window and the data structure
int moveSnake(WINDOW* win, struct pos* snake, int length, int vx, int vy) {
  int result = 0;

  // This if statement prevents the game from writing over the position of 0, 0 when a pellet is eaten
  if(snake[length-1].y) mvwaddch(win, snake[length-1].y, snake[length-1].x, ' ');

  for(int i = length-1; i > 0; i--) {
    snake[i].x = snake[i-1].x;
    snake[i].y = snake[i-1].y;
    mvwaddch(win, snake[i].y, snake[i].x, '#' | COLOR_RED);
  }

  snake[0].x += vx;
  snake[0].y += vy;
  if(mvwinch(win, snake[0].y, snake[0].x) == '#') result = -1;
  else if(mvwinch(win, snake[0].y, snake[0].x) == (PELLET)) result = 1;
  else if(snake[0].y == 0 || snake[0].x == 0 
      || snake[0].x > SIZE || snake[0].y > SIZE) result = -1;

  mvwaddch(win, snake[0].y, snake[0].x, '#' | COLOR_RED);

  return result;
}

int main() {
  struct pos snake[SIZE*SIZE];
  int length = 5;
  int vx = -1, vy = 0;
  int qvx = 0, qvy = 0;
  bool buffer = false;

  int ch;
  clock_t t= clock();

  srand(time(NULL));

  // Build tail to the right of the starting point
  // I trust people not to be dumb and make the tail go past the border
  for(int i = 0; i < length; i++) {
    snake[i].x = 10+i;
    snake[i].y = 10;
  }

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);

  // This should return an error, but will cause the display to appear
  getch();

  // Create new game window
  WINDOW* game = newwin(SIZE+2, SIZE+2, 0, 0);
  wborder(game, 0, 0, 0, 0, 0, 0, 0, 0);

  // Populate the board with the snake and pellet
  moveSnake(game, snake, length, vx, vy);
  placePellet(game);
  wmove(game, snake[0].y, snake[0].x);
  wrefresh(game);

  while(true) {
    if((ch = getch()) != ERR)
      // Buffer assumes you won't enter more than 2 characters in 100ms
      switch(ch) {
        case KEY_UP:
          if(!buffer) {
            vx = 0;
            vy = -1;
            buffer = true;
          } else {
            qvx = 0;
            qvy = -1;
          }
          break;
        case KEY_DOWN:
          if(!buffer) {
            vx = 0;
            vy = 1;
            buffer = true;
          } else {
            qvx = 0;
            qvy = 1;
          }
          break;
        case KEY_LEFT:
          if(!buffer) {
            vx = -1;
            vy = 0;
            buffer = true;
          } else {
            qvx = -1;
            qvy = 0;
          }
          break;
        case KEY_RIGHT:
          if(!buffer) {
            vx = 1;
            vy = 0;
            buffer = true;
          } else {
            qvx = 1;
            qvy = 0;
          }
          break;
      } 
    else if(clock()-t > CLOCKS_PER_SEC/10) {
      t = clock();

      int result;
      if((result = moveSnake(game, snake, length, vx, vy)) == -1) {
        endwin();
        return 0;
      } else if(result) {
        length++;
        placePellet(game);
      }

      wmove(game, snake[0].y, snake[0].x);
      wrefresh(game);

      // Load buffered moves for next turn
      if(qvx|qvy) {
        vx = qvx;
        vy = qvy;
        qvx = 0;
        qvy = 0; 
      } else buffer = false;
    }
  }
}
