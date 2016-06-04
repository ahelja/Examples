typedef enum _GameHeading {
    kGameHeadingLeft = -1,
    kGameHeadingStraight = 0,
    kGameHeadingRight = 1
} GameHeading;

typedef enum _GameDirection {
    kGameDirectionNorth = 0,
    kGameDirectionEast = 1,
    kGameDirectionSouth = 2,
    kGameDirectionWest = 3
} GameDirection;

typedef struct _GamePosition {
    int x;
    int y;
} GamePosition;

typedef enum _GameState {
    kGameStateContinue = 0,
    kGameStateScore = 1,
    kGameStateCrash = 2
} GameState;

GameState worm_guts(GamePosition *wormPositions, unsigned *wormLength, GameDirection *wormDirection, GameHeading *wormHeading, GamePosition *targetPosition, unsigned width, unsigned height, int noBounce);
