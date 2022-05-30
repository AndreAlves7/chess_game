#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>

#define COLOR_BG_WHITE     "\x1b[47m"
#define COLOR_FG_WHITE     "\x1b[37m"

#define COLOR_BG_BRIGHT_WHITE     "\x1b[107m"
#define COLOR_FG_BRIGHT_WHITE     "\x1b[97m"

#define COLOR_BG_BRIGHT_BLACK     "\x1b[100m"

#define COLOR_FG_BLACK     "\x1b[30m"

#define ANSI_COLOR_RESET   "\x1b[0m"


enum Color{
   WHITE = 0,
   BLACK = 1,
};

enum PieceType{
   PAWN = 1,
   KNIGHT = 2,
   BISHOP = 3,
   QUEEN = 4,
   KING = 5,
   ROOK = 6,
   EMPTY = 0,
};

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    enum PieceType type;
    enum Color color;
    Position position;
} Piece;

typedef struct {
    Position target;
    Piece piece;
} Movement;

typedef struct {
    Piece* pieces;
    int count;
} Board;

Board create_empty_board() {
    Board board = {.pieces = NULL, .count=0};
    return board;
}

Board create_starting_board(){
    int i=0;

    Board board = {.pieces = malloc(32 * sizeof(Piece)), .count=32};
    for(int j=0;j<8;j++){
        board.pieces[i++] = (Piece) { .type = PAWN, .color = WHITE, .position = {j, 1} };
    }
    for(int j=0;j<8;j++){
        board.pieces[i++] = (Piece) { .type = PAWN, .color = BLACK, .position = {j, 6} };
    }
    board.pieces[i++] = (Piece) { .type = ROOK, .color = WHITE, .position = {0, 0} };
    board.pieces[i++] = (Piece) { .type = KNIGHT, .color = WHITE, .position = {1, 0} };
    board.pieces[i++] = (Piece) { .type = BISHOP, .color = WHITE, .position = {2, 0} };
    board.pieces[i++] = (Piece) { .type = KING, .color = WHITE, .position = {3, 0} };
    board.pieces[i++] = (Piece) { .type = QUEEN, .color = WHITE, .position = {4, 0} };
    board.pieces[i++] = (Piece) { .type = BISHOP, .color = WHITE, .position = {5, 0} };
    board.pieces[i++] = (Piece) { .type = KNIGHT, .color = WHITE, .position = {6, 0} };
    board.pieces[i++] = (Piece) { .type = ROOK, .color = WHITE, .position = {7, 0} };

    board.pieces[i++] = (Piece) { .type = ROOK, .color = BLACK, .position = {0, 7} };
    board.pieces[i++] = (Piece) { .type = KNIGHT, .color = BLACK, .position = {1, 7} };
    board.pieces[i++] = (Piece) { .type = BISHOP, .color = BLACK, .position = {2, 7} };
    board.pieces[i++] = (Piece) { .type = KING, .color = BLACK, .position = {3, 7} };
    board.pieces[i++] = (Piece) { .type = QUEEN, .color = BLACK, .position = {4, 7} };
    board.pieces[i++] = (Piece) { .type = BISHOP, .color = BLACK, .position = {5, 7} };
    board.pieces[i++] = (Piece) { .type = KNIGHT, .color = BLACK, .position = {6, 7} };
    board.pieces[i++] = (Piece) { .type = ROOK, .color = BLACK, .position = {7, 7} };

    return board;

}

char* type_to_char(enum  PieceType type) {
    switch(type) {
    case KING:
        return "♔";
    case QUEEN:
        return "♕";
    case PAWN:
        return "♙";
    case BISHOP:
        return "♗";
    case KNIGHT:
        return "♘";
    case ROOK:
        return "♖";
    case EMPTY:
        return " ";
    }
}
char* color_to_fgansi(enum  Color color) {
    switch(color) {
    case BLACK:
        return COLOR_FG_BLACK;
    case WHITE:
        return COLOR_FG_BRIGHT_WHITE;
    }
}


int find_piece_index(Board* board, Position position) {
    for(int i = 0; i < board->count; i++){
        Piece* piece = &board->pieces[i];
        if(piece->position.x == position.x && piece->position.y == position.y){
            return i;
        }
    }
    return -1;
}

Piece* find_piece(Board* board, Position position){
    int i = find_piece_index(board, position);
    if (i==-1) {
        return NULL;
    } else {
        return &board->pieces[i];
    }
}

void display_board(Board board) {
    for(int y=0; y < 8 ; y++){
        char* bg_color = ((y % 2 == 0) ? COLOR_BG_WHITE : COLOR_BG_BRIGHT_BLACK);

        for(int x=0; x < 8 ; x++){
            Position position;
            position.x=x;
            position.y=y;

            Piece* piece = find_piece(&board, position);

            printf("%s", bg_color);
            if (piece != NULL) {
                printf("%s%s", color_to_fgansi(piece->color), type_to_char(piece->type));
            } else {
                printf(" ");
            }
            printf(ANSI_COLOR_RESET);

            if (strcmp(bg_color, COLOR_BG_WHITE)==0){
                bg_color = COLOR_BG_BRIGHT_BLACK;
            }else{
                bg_color = COLOR_BG_WHITE;
            }
        }
        printf("\n");
    }
}

void remove_piece(Board* board, Position position) {
    int i = find_piece_index(board, position);
    if (i == -1) {
        return;
    }
    int pieces_to_move = board->count - i - 1;
    if (pieces_to_move > 0) {
        memcpy(&board->pieces[i], &board->pieces[i+1], sizeof(Piece)*pieces_to_move);
    }
    board->count--;
}

void move_piece(Board* board, Movement move){
    remove_piece(board, move.target);
    Position origin = move.piece.position;
    Piece* piece = find_piece(board, origin);
    piece->position = move.target;
}
#include <unistd.h>

void clear_screen()
{
  const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}

typedef struct {
    Position* data;
    int count;
} PositionList;

void append_position(PositionList* positions, Position position) {
    positions->data = realloc(positions->data, sizeof(Position)*(positions->count+1));
    positions->data[positions->count] = position;
    positions->count++;
}


int is_takable(Board* board, Position current, enum Color color) {
    Piece* piece = find_piece(board, current);
    if(piece == NULL || color != piece->color){
        return 1;
    } else {
        return 0;
    }
}


int is_valid_position(Position position) {
    return position.x >= 0 && position.y >= 0 && position.x < 8 && position.y < 8;
}

int append_if_takable(Board* board, Position current, enum Color color, PositionList* valid_targets) {
    Piece* piece = find_piece(board, current);
    if(piece == NULL) {
        append_position(valid_targets, current);
        return 0;
    } else {
        if(piece->color != color){
            append_position(valid_targets, current);
        }
        return 1;
    }
}


void knight_valid_moves(Board* board, Position origin, enum Color color, PositionList* valid_targets){
    Position current;

    current = origin;
    current.x += 2;
    current.y += 1;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }

    current = origin;
    current.x += 2;
    current.y -= 1;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }

    current = origin;
    current.x -= 2;
    current.y += 1;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }

    current = origin;
    current.x -= 2;
    current.y -= 1;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }


   current = origin;
    current.x += 1;
    current.y += 2;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }

    current = origin;
    current.x += 1;
    current.y -= 2;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }

    current = origin;
    current.x -= 1;
    current.y += 2;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }

    current = origin;
    current.x -= 1;
    current.y -= 2;
    if (is_valid_position(current)) {
        append_if_takable(board, current, color, valid_targets);
    }
}

void queen_valid_moves(Board* board, Position origin, enum Color color, PositionList* valid_targets){
    bishop_valid_moves(board, origin, color, valid_targets);
    rook_valid_moves(board, origin, color, valid_targets);
}

void bishop_valid_moves(Board* board, Position origin, enum Color color, PositionList* valid_targets){
    Position current;

    // look for possible moves towards the bottom right
    for(current.x=origin.x+1, current.y=origin.y+1; current.x<8 && current.y<8; current.x++, current.y++){
        if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }

    // look for possible moves towards the bottom left
    for(current.x=origin.x-1, current.y=origin.y+1; current.x>=0 && current.y<8; current.x--, current.y++){
        if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }

    // look for possible moves towards the top left
    for(current.x=origin.x-1, current.y=origin.y-1; current.x>=0 && current.y>=0; current.x--, current.y--){
        if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }

    // look for possible moves towards the top right
    for(current.x=origin.x+1, current.y=origin.y-1; current.x<8 && current.y>=0; current.x++, current.y--){
        if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }
}

void rook_valid_moves(Board* board, Position origin, enum Color color, PositionList* valid_targets){
    Position current;

    // look for possible moves on the right of origin
    current.y = origin.y;
    for(current.x=origin.x+1; current.x<8; current.x++){
        if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }

    // look for possible moves below the origin
    current.x = origin.x;
    for(current.y=origin.y+1;current.y<8;current.y++){
       if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }
    // look for possible moves on the left of origin
    current.y = origin.y;
    for(current.x=origin.x-1;current.x>=0;current.x--){
       if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }
     // look for possible moves above the origin
    current.x = origin.x;
    for(current.y=origin.y-1;current.y>=0;current.y--){
       if(append_if_takable(board, current, color, valid_targets)) {
            break;
        }
    }
}

void print_positionlist(PositionList positions) {
    for(int i=0;i<positions.count;i++) {
        printf("(%d, %d)", positions.data[i].x, positions.data[i].y);
    }
    printf("\n");
}

int main()
{
    SetConsoleOutputCP(65001);

    Board board = create_starting_board();
    display_board(board);
    Movement move;
    move.piece.position.x=0;
    move.piece.position.y=0;
    move.target.x=1;
    move.target.y=1;
    move_piece(&board, move);

    clear_screen();
    display_board(board);

    Position origin = {.x = 3, .y = 1};

    PositionList valid_targets = { .count=0, .data=NULL };
    knight_valid_moves(&board, origin, WHITE, &valid_targets);
    print_positionlist(valid_targets);
}
