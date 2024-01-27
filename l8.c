#include <stdio.h>
#include "life.h"

// You are only allowed to change the contents of this file with respect 
// to improving the performance of this program



// If you need to initialize some data structures for your implementation
// of life then make changes to the following function, otherwise 
// do not change this function.

void initLife() {
}

int life(long oldWorld[N][N], long newWorld[N][N]) {

	int i, j;
	int alive = 0;

	long temp;
	int n = -1, m = -2;
	long *up_row, *bot_row, *cur_row;
	int col1, col2;

	// cur_row = oldWorld[0];
	// bot_row = oldWorld[1];
	// // i = 0  and  j = 0
	// temp = cur_row[1] + cur_row[2] + bot_row[0] + bot_row[1] + oldWorld[2][0];
	// temp = checkHealth(temp, cur_row[0]);
	// newWorld[0][0] = temp;
	// alive += temp;
	// // i = 0  and  j = 1 
	// temp = cur_row[0]+ cur_row[2] + cur_row[3] + bot_row[0] + bot_row[1] + bot_row[2] + oldWorld[2][1];
	// temp = checkHealth(temp, cur_row[0]);
	// newWorld[0][1] = temp;
	// alive += temp;
	// // i = 0  and j = 2~N-3
	// for (j = 2; j < N-2; j++) {
	// 	col1 = j + n;
	// 	col2 = j - n;
	// 	temp = cur_row[j+m] + cur_row[col2] + cur_row[col1] + cur_row[j-m];
	// 	temp += bot_row[col1] + bot_row[j] + bot_row[col2];
	// 	temp += oldWorld[i-m][j];
	// 	temp = checkHealth(temp, cur_row[j]);
	// 	newWorld[i][j] = temp;
	// 	alive += temp;
	// }
	// // i = 0  and j = N-2
	// j = N-3;
	// temp = cur_row[N-4] + cur_row[j] + bot_row[j];
	// j = N-2;
	// temp += bot_row[j] + oldWorld[2][j];
	// j = N-1;
	// temp += cur_row[j] + bot_row[j];
	// temp = checkHealth(temp, cur_row[0]);
	// newWorld[0][1] = temp;
	// alive += temp;

	for (i = 0; i < N; i++) {
		if (i > 0)
			up_row = oldWorld[i + n];
		cur_row = oldWorld[i];
		if (i < N + n)
			bot_row = oldWorld[i - n];

		for (j = 0; j < N; j++) {
			col1 = j + n;
			col2 = j - n;

			temp = 0;
			// // Count the cells to the top left
			// if (i > 0 && j > 0) 
			// 	temp += up_row[col1];
			// // Count the cells immediately above
			// if (i > 0)
			// 	temp += up_row[j];
			// // Count the cells to the top right
			// if (i > 0 && j < N - 1) 
			// 	temp += up_row[col2];
			if (i > 0){
				temp += up_row[j];
				if (j > 0) 
				 	temp += up_row[col1];
				if (j < N - 1)
					temp += up_row[col2];
			}
			// Count the cells to the immediate left
			if (j > 0)
				temp += cur_row[col1];
			// Count the cells to the immediate right
			if (j < N - 1)
				temp += cur_row[col2];
			// Count the cells to the bottom left
			if (j > 0 && i < N - 1)
				temp += bot_row[col1];
			// Count the cells immediately below
			if (i < N - 1)
				temp += bot_row[j];
			// Count the cells to the bottom right
			if (i < N - 1 && j < N - 1)
				temp += bot_row[col2];
			// Count the cells up two rows
			if (i > 1)
				temp += oldWorld[i+m][j];
			// Count the cells down two rows
			if (i < N-2)
				temp += oldWorld[i-m][j];
			// Count the cells left two cols
			if (j > 1)
				temp += cur_row[j+m];
			// Count the cells right two cols
			if (j < N-2)
				temp += cur_row[j-m];

			temp = checkHealth(temp, cur_row[j]);
			newWorld[i][j] = temp;
			alive += temp;
		}
	}

    return alive;
}
