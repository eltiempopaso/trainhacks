int select(int arr[], int n, int k) {
    if (n == 1) {
        return arr[0];
    }

    int pivot = arr[n / 2];

    int lows[n], highs[n], pivots[n];
    int lowCount = 0, highCount = 0, pivotCount = 0;

    for (int i = 0; i < n; i++) {
        if (arr[i] < pivot) {
            lows[lowCount++] = arr[i];
        } else if (arr[i] > pivot) {
            highs[highCount++] = arr[i];
        } else {
            pivots[pivotCount++] = arr[i];
        }
    }

    if (k < lowCount) {
        return select(lows, lowCount, k);
    } else if (k < lowCount + pivotCount) {
        return pivots[0];
    } else {
        return select(highs, highCount, k - lowCount - pivotCount);
    }
}

float torben_mogensen_median(int arr[], int n) {
    if (n % 2 == 1) {
        return select(arr, n, n / 2);
    } else {
        return 0.5 * (select(arr, n, n / 2 - 1) + select(arr, n, n / 2));
    }
}
