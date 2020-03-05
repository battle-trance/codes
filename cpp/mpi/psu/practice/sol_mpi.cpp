#include <cstdio>
#include <mpi.h>
#include <cstdlib>
#include <ctime>

const int N = 50;
const int MASTER = 0;

int a[N];

int sumDivs(int x) {
  int sum = 0;
  for (int i = 1; i * i <= x; ++i) {
    if (x % i == 0) {
      sum += i;
      if (i * i != x) {
        sum += x / i;
      }
    }
  }
  return sum - x;
}

void doWork(int offset, int chunkSize, int& sum, int& count) {
  for (int i = offset; i < offset + chunkSize; ++i) {
    if (a[i] == sumDivs(a[i])) {
      sum += a[i];
      count++;
    }
  }
}

int main(int argc, char** argv) {

  srand(time(0));

  int size;
  int rank;
  int chunkSize = 0;
  int leftOver = 0;
  int offset = 0;
  int tag1 = 1;
  int tag2 = 2;

  int count = 0;
  int myCount = 0;
  int sum = 0;
  int mySum = 0;

  
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  chunkSize = N / size;
  leftOver = N % size;

  if (rank == MASTER) {
  
    for (int i = 0; i < N; ++i) {
      a[i] = 1 + rand() % 50;
    }

    printf("Сгенерированный массив:\n");
    for (int i = 0; i < N; ++i) {
      if (i) {
        printf(" ");
      }
      printf("%d", a[i]);
    }
    printf("\n");

    printf("Совершенные элементы массива: \n");
    for (int i = 0; i < N; ++i) {
      if (a[i] == sumDivs(a[i])) {
        printf("%d ", a[i]);
      }
    }
    printf("\n");

    offset = chunkSize + leftOver;
    for (int dest = 1; dest < size; ++dest) {
      MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
      MPI_Send(&a[offset], chunkSize, MPI_INT, dest, tag2, MPI_COMM_WORLD);
      offset = offset + chunkSize;
    }


    offset = 0;
    doWork(offset, chunkSize + leftOver, mySum, myCount);

    for (int source = 1; source < size; ++source) {
      MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
      MPI_Recv(&a[offset], chunkSize, MPI_INT, source, tag2, MPI_COMM_WORLD, &status);
    }


    MPI_Reduce(&mySum, &sum, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&myCount, &count, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

    if (count) {
      printf("Среднее арифметическое совершенных элементов: %lf\n", (double) sum / count);
    }
    else {
      printf("Совершенных элементов в массиве нет\n");
    }

  }


  if (rank != MASTER) {

    MPI_Recv(&offset, 1, MPI_INT, MASTER, tag1, MPI_COMM_WORLD, &status);
    MPI_Recv(&a[offset], chunkSize, MPI_INT, MASTER, tag2, MPI_COMM_WORLD, &status);

    doWork(offset, chunkSize, mySum, myCount);

    MPI_Send(&offset, 1, MPI_INT, MASTER, tag1, MPI_COMM_WORLD);
    MPI_Send(&a[offset], chunkSize, MPI_INT, MASTER, tag2, MPI_COMM_WORLD);

    MPI_Reduce(&mySum, &sum, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&myCount, &count, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

  }

  MPI_Finalize();
}
