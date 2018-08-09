int printf(char *str, int a, int b);

int validate(int *map, int N, int p)
{
    if (map[p]) return 0;

    int row = p / N, col = p % N;
    for (int i = 0; i < N; i++)
        if (map[i * N + col]) return 0;
    for (int i = 0; i < N; i++)
        if (map[row * N + i]) return 0;

    // diagonal
    for (int i = row, j = col; 0 <= i && i < N && 0 <= j && j < N; i++, j++)
        if (map[i * N + j]) return 0;
    for (int i = row, j = col; 0 <= i && i < N && 0 <= j && j < N; i--, j--)
        if (map[i * N + j]) return 0;
    for (int i = row, j = col; 0 <= i && i < N && 0 <= j && j < N; i++, j--)
        if (map[i * N + j]) return 0;
    for (int i = row, j = col; 0 <= i && i < N && 0 <= j && j < N; i--, j++)
        if (map[i * N + j]) return 0;

    return 1;
}

int dump(int *map, int N)
{
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) printf("%d ", map[i * N + j], N);
        printf("\n", N, N);
    }
    printf("\n", N, N);
}

int put(int *map, int N, int row, int *res)
{
    if (row == N) {
        (*res)++;
        dump(map, N);
    }

    int i;
    for (i = 0; i < N; i++) {
        int p = row * N + i;
        if (!validate(map, N, p)) continue;
        map[p] = 1;
        put(map, N, row + 1, res);
        map[p] = 0;
    }
}

int nqueen(int N)
{
    int i, map[100];
    for (i = 0; i < N * N; i++) map[i] = 0;  // init

    int res = 0;
    put(map, N, 0, &res);
    return res;
}

int main()
{
    int i;
    for (i = 1; i <= 8; i++) printf("%d: %d\n", i, nqueen(i));
}
