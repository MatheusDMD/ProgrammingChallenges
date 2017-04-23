/* mobility.hpp - Function prototypes

*/

/* Binary-based (4-based) mob table: Two bits per square, 1=01=B, 2=10=W, 0=00=E.
  Used for bitmap indexing. */
extern unsigned short mobTableBin[1 << 16];

/* Initialize the binary table from the tertiary one.
  Run once only each execution of the program*/
void initMobTableBinFrom(unsigned short *mt3);

/* Initialized and print out the 3-based mobility table as a .hpp file.
  Only do it once and for all. */
void genMobTable();
