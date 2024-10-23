#include<bits/stdc++.h>
using namespace std;

void printArray(const vector<int> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

// Bubble Sort
void bubbleSort(vector<int> &arr) {
    int n = arr.size();
    for (int i = 0; i < n-1; i++) {
        cout << "Pass " << i + 1 << ": " << endl;
        for (int j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                swap(arr[j], arr[j+1]);
            }
            printArray(arr);
        }
    }
}

// Selection Sort
void selectionSort(vector<int> &arr) {
    int n = arr.size();
    for (int i = 0; i < n-1; i++) {
        cout << "Pass " << i + 1 << ": " << endl;
        int min_idx = i;
        for (int j = i+1; j < n; j++) {
            if (arr[j] < arr[min_idx])
                min_idx = j;
        }
        swap(arr[i], arr[min_idx]);
        printArray(arr);
    }
}

// Insertion Sort
void insertionSort(vector<int> &arr) {
    int n = arr.size();
    for (int i = 1; i < n; i++) {
        cout << "Pass " << i << ": " << endl;
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
            printArray(arr);
        }
        arr[j + 1] = key;
        printArray(arr);
    }
}

// Merge Sort
void merge(vector<int> &arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int i = 0; i < n2; i++)
        R[i] = arr[m + 1 + i];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
    printArray(arr);
}

void mergeSort(vector<int> &arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

// Quick Sort
int partition(vector<int> &arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
            printArray(arr);
        }
    }
    swap(arr[i + 1], arr[high]);
    printArray(arr);
    return (i + 1);
}

void quickSort(vector<int> &arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// Heap Sort
void heapify(vector<int> &arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        swap(arr[i], arr[largest]);
        printArray(arr);
        heapify(arr, n, largest);
    }
}

void heapSort(vector<int> &arr) {
    int n = arr.size();
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    for (int i = n - 1; i > 0; i--) {
        swap(arr[0], arr[i]);
        printArray(arr);
        heapify(arr, i, 0);
    }
}

// Counting Sort
void countingSort(vector<int> &arr) {
    int max = *max_element(arr.begin(), arr.end());
    vector<int> count(max + 1, 0);

    for (int i = 0; i < arr.size(); i++)
        count[arr[i]]++;

    int index = 0;
    for (int i = 0; i <= max; i++) {
        while (count[i]--) {
            arr[index++] = i;
            printArray(arr);
        }
    }
}

// Radix Sort
int getMax(vector<int> &arr) {
    return *max_element(arr.begin(), arr.end());
}

void countingSortForRadix(vector<int> &arr, int exp) {
    int n = arr.size();
    vector<int> output(n);
    vector<int> count(10, 0);

    for (int i = 0; i < n; i++)
        count[(arr[i] / exp) % 10]++;

    for (int i = 1; i < 10; i++)
        count[i] += count[i - 1];

    for (int i = n - 1; i >= 0; i--) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }

    for (int i = 0; i < n; i++) {
        arr[i] = output[i];
        printArray(arr);
    }
}

void radixSort(vector<int> &arr) {
    int m = getMax(arr);
    for (int exp = 1; m / exp > 0; exp *= 10)
        countingSortForRadix(arr, exp);
}

// Bucket Sort
void bucketSort(vector<int> &arr) {
    int n = arr.size();
    int bucketCount = sqrt(n);
    vector<vector<int>> buckets(bucketCount);

    int maxValue = *max_element(arr.begin(), arr.end());
    for (int i = 0; i < n; i++) {
        int bucketIndex = bucketCount * arr[i] / (maxValue + 1);
        buckets[bucketIndex].push_back(arr[i]);
    }

    int index = 0;
    for (int i = 0; i < bucketCount; i++) {
        sort(buckets[i].begin(), buckets[i].end());
        for (int j = 0; j < buckets[i].size(); j++) {
            arr[index++] = buckets[i][j];
            printArray(arr);
        }
    }
}

// Shell Sort
void shellSort(vector<int> &arr) {
    int n = arr.size();
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) {
                arr[j] = arr[j - gap];
                printArray(arr);
            }
            arr[j] = temp;
            printArray(arr);
        }
    }
}

int main() {
    int n;
    cout << "Enter the number of elements in the array : ";
    cin >> n;
    cout << "Enter the elements : \n";
    vector<int> arr(n);
    for(int i = 0; i<n; i++){
        cin >> arr[i];
    }
    cout << endl;
    int choice;

    cout << "Original Array: ";
    printArray(arr);

    cout << "\nSelect Sorting Algorithm:\n";
    cout << "1. Bubble Sort\n";
    cout << "2. Selection Sort\n";
    cout << "3. Insertion Sort\n";
    cout << "4. Merge Sort\n";
    cout << "5. Quick Sort\n";
    cout << "6. Heap Sort\n";
    cout << "7. Counting Sort\n";
    cout << "8. Radix Sort\n";
    cout << "9. Bucket Sort\n";
    cout << "10. Shell Sort\n";
    cout << "\nEnter your choice: ";
    cin >> choice;
    cout << endl;

    switch (choice) {
        case 1: bubbleSort(arr); break;
        case 2: selectionSort(arr); break;
        case 3: insertionSort(arr); break;
        case 4: mergeSort(arr, 0, arr.size() - 1); break;
        case 5: quickSort(arr, 0, arr.size() - 1); break;
        case 6: heapSort(arr); break;
        case 7: countingSort(arr); break;
        case 8: radixSort(arr); break;
        case 9: bucketSort(arr); break;
        case 10: shellSort(arr); break;
        default: cout << "Invalid choice!\n";
    }

    cout << "\nSorted Array: ";
    printArray(arr);

    return 0;
}
