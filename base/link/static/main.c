#define LEN 2
extern int sharedArr[LEN];
extern int sum(int *arr, int n);
int* array = sharedArr;

int main(void)
{
    int val = sum(array, LEN);
    return val;
}