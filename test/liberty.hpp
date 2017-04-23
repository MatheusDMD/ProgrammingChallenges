/* liberty.hpp  -- Prototypes for liberty.cpp

*/

// Liberty map
extern char libmap[91];
extern int pMobDiff; // BLACK - WHITE

/* Initialize the map given a 91-entry array board representation
*/
void initLibmap(char *a);

/* Update the liberty map after 'move' is played */
void updateLibmap(int move);

/* Update the liberty map after 'move' is undone */
void updateLibmapUndo(int move);
