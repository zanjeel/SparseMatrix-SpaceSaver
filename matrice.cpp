#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <vector>

// Node structure for matrix elements
struct MatrixNode {
    int col;            // Column index
    double value;       // Value at this position
    MatrixNode* next;   // Next element in this row
    
    MatrixNode(int c, double v) : col(c), value(v), next(nullptr) {}
};

// Row structure for matrix rows
struct RowNode {
    int row;            // Row index
    MatrixNode* elements; // Linked list of elements in this row
    RowNode* next;      // Next row in the matrix
    
    RowNode(int r) : row(r), elements(nullptr), next(nullptr) {}
    
    // Destructor to clean up all column nodes
    ~RowNode() {
        MatrixNode* current = elements;
        while (current != nullptr) {
            MatrixNode* temp = current;
            current = current->next;
            delete temp;
        }
    }
};

// Sparse Matrix class using linked lists
class SparseMatrix {
private:
    int rows;           // Number of rows
    int cols;           // Number of columns
    RowNode* rowList;   // Linked list of rows
    
    // Helper function to get a row node (creates it if it doesn't exist)
    RowNode* getRowNode(int r, bool create = false) {
        if (r < 0 || r >= rows) {
            throw std::out_of_range("Row index out of range");
        }
        
        // If rowList is empty, create first row node
        if (rowList == nullptr && create) {
            rowList = new RowNode(r);
            return rowList;
        }
        
        // If the first row is greater than r and we need to create, insert at beginning
        if (rowList != nullptr && rowList->row > r && create) {
            RowNode* newRow = new RowNode(r);
            newRow->next = rowList;
            rowList = newRow;
            return newRow;
        }
        
        // Look for existing row or place to insert
        RowNode* prev = nullptr;
        RowNode* current = rowList;
        
        while (current != nullptr && current->row < r) {
            prev = current;
            current = current->next;
        }
        
        // Found existing row
        if (current != nullptr && current->row == r) {
            return current;
        }
        
        // Need to create a new row node
        if (create) {
            RowNode* newRow = new RowNode(r);
            if (prev == nullptr) {
                // Insert at start (should not happen due to checks above)
                newRow->next = rowList;
                rowList = newRow;
            } else {
                // Insert between prev and current
                newRow->next = current;
                prev->next = newRow;
            }
            return newRow;
        }
        
        return nullptr; // Row doesn't exist and we're not creating it
    }
    
    // Helper function to insert an element into a row's linked list
    void insertIntoRow(RowNode* rowNode, int c, double v) {
        if (c < 0 || c >= cols) {
            throw std::out_of_range("Column index out of range");
        }
        
        // If value is zero, we might need to remove existing element
        if (std::abs(v) < 1e-10) {
            removeFromRow(rowNode, c);
            return;
        }
        
        // If row has no elements, create first element
        if (rowNode->elements == nullptr) {
            rowNode->elements = new MatrixNode(c, v);
            return;
        }
        
        // If first element's column is greater than c, insert at beginning
        if (rowNode->elements->col > c) {
            MatrixNode* newNode = new MatrixNode(c, v);
            newNode->next = rowNode->elements;
            rowNode->elements = newNode;
            return;
        }
        
        // Look for existing column or place to insert
        MatrixNode* prev = nullptr;
        MatrixNode* current = rowNode->elements;
        
        while (current != nullptr && current->col < c) {
            prev = current;
            current = current->next;
        }
        
        // Found existing column, update value
        if (current != nullptr && current->col == c) {
            current->value = v;
            return;
        }
        
        // Insert new node between prev and current
        MatrixNode* newNode = new MatrixNode(c, v);
        if (prev == nullptr) {
            // Should not reach here due to checks above
            newNode->next = rowNode->elements;
            rowNode->elements = newNode;
        } else {
            newNode->next = current;
            prev->next = newNode;
        }
    }
    
    // Helper function to remove an element from a row
    void removeFromRow(RowNode* rowNode, int c) {
        if (rowNode == nullptr || rowNode->elements == nullptr) {
            return;
        }
        
        // If first element is the one to remove
        if (rowNode->elements->col == c) {
            MatrixNode* temp = rowNode->elements;
            rowNode->elements = rowNode->elements->next;
            delete temp;
            return;
        }
        
        // Look for the element to remove
        MatrixNode* prev = rowNode->elements;
        MatrixNode* current = prev->next;
        
        while (current != nullptr && current->col != c) {
            prev = current;
            current = current->next;
        }
        
        // If found, remove it
        if (current != nullptr) {
            prev->next = current->next;
            delete current;
        }
    }
    
    // Helper function to clean up empty rows
    void cleanupEmptyRows() {
        if (rowList == nullptr) {
            return;
        }
        
        // Check if first row is empty
        while (rowList != nullptr && rowList->elements == nullptr) {
            RowNode* temp = rowList;
            rowList = rowList->next;
            delete temp;
        }
        
        if (rowList == nullptr) {
            return;
        }
        
        // Check remaining rows
        RowNode* prev = rowList;
        RowNode* current = prev->next;
        
        while (current != nullptr) {
            if (current->elements == nullptr) {
                prev->next = current->next;
                delete current;
                current = prev->next;
            } else {
                prev = current;
                current = current->next;
            }
        }
    }
    
public:
    // Constructor
    SparseMatrix(int r, int c) : rows(r), cols(c), rowList(nullptr) {
        if (r <= 0 || c <= 0) {
            throw std::invalid_argument("Matrix dimensions must be positive");
        }
    }
    
    // Copy constructor
    SparseMatrix(const SparseMatrix& other) : rows(other.rows), cols(other.cols), rowList(nullptr) {
        // Deep copy each row and its elements
        RowNode* otherRow = other.rowList;
        RowNode* lastRow = nullptr;
        
        while (otherRow != nullptr) {
            // Create a new row
            RowNode* newRow = new RowNode(otherRow->row);
            
            // Add to our row list
            if (lastRow == nullptr) {
                rowList = newRow;
            } else {
                lastRow->next = newRow;
            }
            lastRow = newRow;
            
            // Copy all elements in this row
            MatrixNode* otherElement = otherRow->elements;
            MatrixNode* lastElement = nullptr;
            
            while (otherElement != nullptr) {
                MatrixNode* newElement = new MatrixNode(otherElement->col, otherElement->value);
                
                // Add to our element list
                if (lastElement == nullptr) {
                    newRow->elements = newElement;
                } else {
                    lastElement->next = newElement;
                }
                lastElement = newElement;
                
                otherElement = otherElement->next;
            }
            
            otherRow = otherRow->next;
        }
    }
    
    // Destructor
    ~SparseMatrix() {
        RowNode* current = rowList;
        while (current != nullptr) {
            RowNode* temp = current;
            current = current->next;
            delete temp;
        }
    }
    
    // Assignment operator
    SparseMatrix& operator=(const SparseMatrix& other) {
        if (this != &other) {
            // Clear existing data
            RowNode* current = rowList;
            while (current != nullptr) {
                RowNode* temp = current;
                current = current->next;
                delete temp;
            }
            
            // Copy from other
            rows = other.rows;
            cols = other.cols;
            rowList = nullptr;
            
            // Deep copy rows and elements (same as copy constructor)
            RowNode* otherRow = other.rowList;
            RowNode* lastRow = nullptr;
            
            while (otherRow != nullptr) {
                RowNode* newRow = new RowNode(otherRow->row);
                
                if (lastRow == nullptr) {
                    rowList = newRow;
                } else {
                    lastRow->next = newRow;
                }
                lastRow = newRow;
                
                MatrixNode* otherElement = otherRow->elements;
                MatrixNode* lastElement = nullptr;
                
                while (otherElement != nullptr) {
                    MatrixNode* newElement = new MatrixNode(otherElement->col, otherElement->value);
                    
                    if (lastElement == nullptr) {
                        newRow->elements = newElement;
                    } else {
                        lastElement->next = newElement;
                    }
                    lastElement = newElement;
                    
                    otherElement = otherElement->next;
                }
                
                otherRow = otherRow->next;
            }
        }
        return *this;
    }
    
    // Get dimensions
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    
    // Insert an element (r, c) with value v
    void insert(int r, int c, double v) {
        // If value is 0, we might need to remove an existing element
        if (std::abs(v) < 1e-10) {
            RowNode* rowNode = getRowNode(r);
            if (rowNode != nullptr) {
                removeFromRow(rowNode, c);
                if (rowNode->elements == nullptr) {
                    cleanupEmptyRows();
                }
            }
            return;
        }
        
        // Get or create the row node
        RowNode* rowNode = getRowNode(r, true);
        insertIntoRow(rowNode, c, v);
    }
    
    // Get value at position (r, c)
    double get(int r, int c) const {
        if (r < 0 || r >= rows || c < 0 || c >= cols) {
            throw std::out_of_range("Index out of range");
        }
        
        // Find the row
        RowNode* rowNode = rowList;
        while (rowNode != nullptr && rowNode->row < r) {
            rowNode = rowNode->next;
        }
        
        // Row not found, return 0
        if (rowNode == nullptr || rowNode->row != r) {
            return 0.0;
        }
        
        // Find the column in this row
        MatrixNode* colNode = rowNode->elements;
        while (colNode != nullptr && colNode->col < c) {
            colNode = colNode->next;
        }
        
        // Column not found, return 0
        if (colNode == nullptr || colNode->col != c) {
            return 0.0;
        }
        
        return colNode->value;
    }
    
    // Display the matrix
    void display() const {
        std::cout << "Matrix " << rows << "x" << cols << ":" << std::endl;
        
        // Check if matrix is entirely zero
        if (rowList == nullptr) {
            std::cout << "Empty matrix (all zeros)" << std::endl;
            return;
        }
        
        // Display full matrix
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                std::cout << std::setw(8) << std::fixed << std::setprecision(2) << get(i, j) << " ";
            }
            std::cout << std::endl;
        }
    }
    
    // Display sparse representation
    void displaySparse() const {
        std::cout << "Sparse representation of " << rows << "x" << cols << " matrix:" << std::endl;
        std::cout << "Row\tColumn\tValue" << std::endl;
        
        int nonZeroCount = 0;
        RowNode* rowNode = rowList;
        
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                std::cout << rowNode->row << "\t" << colNode->col << "\t" 
                          << std::fixed << std::setprecision(2) << colNode->value << std::endl;
                nonZeroCount++;
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        
        std::cout << "Total non-zero elements: " << nonZeroCount << std::endl;
    }
    
    // Addition with another matrix
    SparseMatrix add(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::invalid_argument("Matrix dimensions do not match for addition");
        }
        
        SparseMatrix result(rows, cols);
        
        // Add all elements from this matrix
        RowNode* rowNode = rowList;
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                result.insert(rowNode->row, colNode->col, colNode->value);
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        
        // Add all elements from other matrix
        rowNode = other.rowList;
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                double currentValue = result.get(rowNode->row, colNode->col);
                result.insert(rowNode->row, colNode->col, currentValue + colNode->value);
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        
        return result;
    }
    
    // Subtraction with another matrix
    SparseMatrix subtract(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::invalid_argument("Matrix dimensions do not match for subtraction");
        }
        
        SparseMatrix result(rows, cols);
        
        // Add all elements from this matrix
        RowNode* rowNode = rowList;
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                result.insert(rowNode->row, colNode->col, colNode->value);
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        
        // Subtract all elements from other matrix
       // Subtract all elements from other matrix
        rowNode = other.rowList;
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                double currentValue = result.get(rowNode->row, colNode->col);
                result.insert(rowNode->row, colNode->col, currentValue - colNode->value);
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        
        return result;
    }
    
    // Scalar multiplication
    SparseMatrix scalarMultiply(double scalar) const {
        SparseMatrix result(rows, cols);
        
        if (std::abs(scalar) < 1e-10) {
            return result; // Return empty matrix if scalar is zero
        }
        
        RowNode* rowNode = rowList;
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                double newValue = colNode->value * scalar;
                if (std::abs(newValue) >= 1e-10) {
                    result.insert(rowNode->row, colNode->col, newValue);
                }
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        
        return result;
    }
    
    // Matrix multiplication
    SparseMatrix multiply(const SparseMatrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication");
        }
        
        SparseMatrix result(rows, other.cols);
        
        // For each row in this matrix
        RowNode* rowNode = rowList;
        while (rowNode != nullptr) {
            int i = rowNode->row;
            
            // For each column in the result matrix
            for (int j = 0; j < other.cols; j++) {
                double sum = 0.0;
                
                // For each element in this row
                MatrixNode* colNode = rowNode->elements;
                while (colNode != nullptr) {
                    int k = colNode->col;
                    double val1 = colNode->value;
                    double val2 = other.get(k, j);
                    
                    if (std::abs(val2) >= 1e-10) {
                        sum += val1 * val2;
                    }
                    
                    colNode = colNode->next;
                }
                
                if (std::abs(sum) >= 1e-10) {
                    result.insert(i, j, sum);
                }
            }
            
            rowNode = rowNode->next;
        }
        
        return result;
    }
    
    // Scalar division
    SparseMatrix scalarDivide(double scalar) const {
        if (std::abs(scalar) < 1e-10) {
            throw std::invalid_argument("Division by zero");
        }
        
        return scalarMultiply(1.0 / scalar);
    }
    
    // Transpose of matrix
    SparseMatrix transpose() const {
        SparseMatrix result(cols, rows);
        
        RowNode* rowNode = rowList;
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                result.insert(colNode->col, rowNode->row, colNode->value);
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        
        return result;
    }
    
    // Calculate determinant (only for 2x2 and 3x3 matrices)
    double determinant() const {
        if (rows != cols) {
            throw std::invalid_argument("Matrix must be square to calculate determinant");
        }
        
        if (rows == 1) {
            return get(0, 0);
        } else if (rows == 2) {
            return get(0, 0) * get(1, 1) - get(0, 1) * get(1, 0);
        } else if (rows == 3) {
            double a = get(0, 0);
            double b = get(0, 1);
            double c = get(0, 2);
            double d = get(1, 0);
            double e = get(1, 1);
            double f = get(1, 2);
            double g = get(2, 0);
            double h = get(2, 1);
            double i = get(2, 2);
            
            return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
        } else {
            throw std::invalid_argument("Determinant calculation for matrices larger than 3x3 not implemented");
        }
    }
    
    // Calculate inverse (only for 2x2 and 3x3 matrices)
    SparseMatrix inverse() const {
        if (rows != cols) {
            throw std::invalid_argument("Matrix must be square to calculate inverse");
        }
        
        double det = determinant();
        if (std::abs(det) < 1e-10) {
            throw std::invalid_argument("Matrix is singular, inverse does not exist");
        }
        
        SparseMatrix result(rows, cols);
        
        if (rows == 1) {
            result.insert(0, 0, 1.0 / get(0, 0));
        } else if (rows == 2) {
            result.insert(0, 0, get(1, 1) / det);
            result.insert(0, 1, -get(0, 1) / det);
            result.insert(1, 0, -get(1, 0) / det);
            result.insert(1, 1, get(0, 0) / det);
        } else if (rows == 3) {
            double a = get(0, 0);
            double b = get(0, 1);
            double c = get(0, 2);
            double d = get(1, 0);
            double e = get(1, 1);
            double f = get(1, 2);
            double g = get(2, 0);
            double h = get(2, 1);
            double i = get(2, 2);
            
            // Calculate cofactor matrix
            double A = e * i - f * h;
            double B = -(d * i - f * g);
            double C = d * h - e * g;
            double D = -(b * i - c * h);
            double E = a * i - c * g;
            double F = -(a * h - b * g);
            double G = b * f - c * e;
            double H = -(a * f - c * d);
            double I = a * e - b * d;
            
            // Adjugate (transpose of cofactor matrix)
            result.insert(0, 0, A / det);
            result.insert(0, 1, D / det);
            result.insert(0, 2, G / det);
            result.insert(1, 0, B / det);
            result.insert(1, 1, E / det);
            result.insert(1, 2, H / det);
            result.insert(2, 0, C / det);
            result.insert(2, 1, F / det);
            result.insert(2, 2, I / det);
        } else {
            throw std::invalid_argument("Inverse calculation for matrices larger than 3x3 not implemented");
        }
        
        return result;
    }
    
    // Count non-zero elements
    int countNonZero() const {
        int count = 0;
        RowNode* rowNode = rowList;
        while (rowNode != nullptr) {
            MatrixNode* colNode = rowNode->elements;
            while (colNode != nullptr) {
                count++;
                colNode = colNode->next;
            }
            rowNode = rowNode->next;
        }
        return count;
    }
};

// Function to read a matrix from user input
SparseMatrix readMatrix() {
    int rows, cols;
    std::cout << "Enter number of rows: ";
    std::cin >> rows;
    std::cout << "Enter number of columns: ";
    std::cin >> cols;
    
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Dimensions must be positive");
    }
    
    SparseMatrix matrix(rows, cols);
    
    std::cout << "Enter matrix elements row by row:" << std::endl;
    for (int i = 0; i < rows; i++) {
        std::cout << "Row " << i << ":" << std::endl;
        for (int j = 0; j < cols; j++) {
            double value;
            std::cout << "Element at position (" << i << ", " << j << "): ";
            std::cin >> value;
            if (std::abs(value) >= 1e-10) {  // Only insert non-zero values
                matrix.insert(i, j, value);
            }
        }
    }
    
    return matrix;
}

// Function to run tests
void runTests() {
    std::cout << "=== RUNNING TESTS ===" << std::endl;
    
    // Test 1: Addition
    std::cout << "Test 1: Addition" << std::endl;
    SparseMatrix m1(2, 2);
    m1.insert(0, 0, 1);
    m1.insert(0, 1, 2);
    m1.insert(1, 0, 3);
    m1.insert(1, 1, 4);
    
    SparseMatrix m2(2, 2);
    m2.insert(0, 0, 5);
    m2.insert(0, 1, 6);
    m2.insert(1, 0, 7);
    m2.insert(1, 1, 8);
    
    std::cout << "Matrix 1:" << std::endl;
    m1.display();
    std::cout << "Matrix 2:" << std::endl;
    m2.display();
    
    SparseMatrix mAdd = m1.add(m2);
    std::cout << "Result of addition:" << std::endl;
    mAdd.display();
    std::cout << std::endl;
    
    // Test 2: Subtraction
    std::cout << "Test 2: Subtraction" << std::endl;
    SparseMatrix mSub = m1.subtract(m2);
    std::cout << "Result of subtraction (M1 - M2):" << std::endl;
    mSub.display();
    std::cout << std::endl;
    
    // Test 3: Scalar multiplication
    std::cout << "Test 3: Scalar multiplication" << std::endl;
    SparseMatrix mScalarMult = m1.scalarMultiply(2.5);
    std::cout << "Result of M1 * 2.5:" << std::endl;
    mScalarMult.display();
    std::cout << std::endl;
    
    // Test 4: Matrix multiplication
    std::cout << "Test 4: Matrix multiplication" << std::endl;
    SparseMatrix mMult = m1.multiply(m2);
    std::cout << "Result of M1 * M2:" << std::endl;
    mMult.display();
    std::cout << std::endl;
    
    // Test 5: Transpose
    std::cout << "Test 5: Transpose" << std::endl;
    SparseMatrix mTrans = m1.transpose();
    std::cout << "Transpose of M1:" << std::endl;
    mTrans.display();
    std::cout << std::endl;
    
    // Test 6: Determinant
    std::cout << "Test 6: Determinant" << std::endl;
    double det = m1.determinant();
    std::cout << "Determinant of M1: " << det << std::endl << std::endl;
    
    // Test 7: Inverse
    std::cout << "Test 7: Inverse" << std::endl;
    try {
        SparseMatrix mInv = m1.inverse();
        std::cout << "Inverse of M1:" << std::endl;
        mInv.display();
        
        // Verify: M * M^-1 = I
        std::cout << "Verification M1 * M1^-1:" << std::endl;
        SparseMatrix mVerify = m1.multiply(mInv);
        mVerify.display();
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl << std::endl;
    }
    
    // Test 8: Sparse representation
    std::cout << "Test 8: Sparse representation" << std::endl;
    SparseMatrix m3(3, 3);
    m3.insert(0, 0, 1);
    m3.insert(0, 2, 3);
    m3.insert(2, 1, 7);
    
    std::cout << "Matrix:" << std::endl;
    m3.display();
    std::cout << "Sparse representation:" << std::endl;
    m3.displaySparse();
    std::cout << std::endl;
}

// Main menu function
void displayMenu() {
    std::cout << "\n=== SPARSE MATRIX CALCULATOR ===" << std::endl;
    std::cout << "1. Create a new matrix" << std::endl;
    std::cout << "2. Add two matrices" << std::endl;
    std::cout << "3. Subtract two matrices" << std::endl;
    std::cout << "4. Multiply by scalar" << std::endl;
    std::cout << "5. Multiply two matrices" << std::endl;
    std::cout << "6. Divide by scalar" << std::endl;
    std::cout << "7. Transpose a matrix" << std::endl;
    std::cout << "8. Calculate determinant" << std::endl;
    std::cout << "9. Calculate inverse" << std::endl;
    std::cout << "10. View matrix" << std::endl;
    std::cout << "11. View sparse representation" << std::endl;
    std::cout << "12. Run tests" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

int main() {
    std::vector<SparseMatrix> matrices;
    int choice = -1;
    
    while (choice != 0) {
        displayMenu();
        std::cin >> choice;
        
        try {
            switch (choice) {
                case 1: {  // Create a new matrix
                    matrices.push_back(readMatrix());
                    std::cout << "Matrix " << matrices.size() - 1 << " created successfully." << std::endl;
                    break;
                }
                case 2: {  // Add two matrices
                    if (matrices.size() < 2) {
                        std::cout << "You need at least two matrices. Create more matrices." << std::endl;
                        break;
                    }
                    
                    int idx1, idx2;
                    std::cout << "Enter index of first matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx1;
                    std::cout << "Enter index of second matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx2;
                    
                    if (idx1 < 0 || idx1 >= matrices.size() || idx2 < 0 || idx2 >= matrices.size()) {
std::cout << "Invalid matrix indices." << std::endl;
                        break;
                    }
                    
                    SparseMatrix result = matrices[idx1].add(matrices[idx2]);
                    matrices.push_back(result);
                    std::cout << "Result stored as matrix " << matrices.size() - 1 << std::endl;
                    result.display();
                    break;
                }
                case 3: {  // Subtract two matrices
                    if (matrices.size() < 2) {
                        std::cout << "You need at least two matrices. Create more matrices." << std::endl;
                        break;
                    }
                    
                    int idx1, idx2;
                    std::cout << "Enter index of first matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx1;
                    std::cout << "Enter index of second matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx2;
                    
                    if (idx1 < 0 || idx1 >= matrices.size() || idx2 < 0 || idx2 >= matrices.size()) {
                        std::cout << "Invalid matrix indices." << std::endl;
                        break;
                    }
                    
                    SparseMatrix result = matrices[idx1].subtract(matrices[idx2]);
                    matrices.push_back(result);
                    std::cout << "Result stored as matrix " << matrices.size() - 1 << std::endl;
                    result.display();
                    break;
                }
                case 4: {  // Multiply by scalar
                    if (matrices.empty()) {
                        std::cout << "No matrices available. Create a matrix first." << std::endl;
                        break;
                    }
                    
                    int idx;
                    double scalar;
                    std::cout << "Enter index of matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx;
                    std::cout << "Enter scalar value: ";
                    std::cin >> scalar;
                    
                    if (idx < 0 || idx >= matrices.size()) {
                        std::cout << "Invalid matrix index." << std::endl;
                        break;
                    }
                    
                    SparseMatrix result = matrices[idx].scalarMultiply(scalar);
                    matrices.push_back(result);
                    std::cout << "Result stored as matrix " << matrices.size() - 1 << std::endl;
                    result.display();
                    break;
                }
                case 5: {  // Multiply two matrices
                    if (matrices.size() < 2) {
                        std::cout << "You need at least two matrices. Create more matrices." << std::endl;
                        break;
                    }
                    
                    int idx1, idx2;
                    std::cout << "Enter index of first matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx1;
                    std::cout << "Enter index of second matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx2;
                    
                    if (idx1 < 0 || idx1 >= matrices.size() || idx2 < 0 || idx2 >= matrices.size()) {
                        std::cout << "Invalid matrix indices." << std::endl;
                        break;
                    }
                    
                    SparseMatrix result = matrices[idx1].multiply(matrices[idx2]);
                    matrices.push_back(result);
                    std::cout << "Result stored as matrix " << matrices.size() - 1 << std::endl;
                    result.display();
                    break;
                }
                case 6: {  // Divide by scalar
                    if (matrices.empty()) {
                        std::cout << "No matrices available. Create a matrix first." << std::endl;
                        break;
                    }
                    
                    int idx;
                    double scalar;
                    std::cout << "Enter index of matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx;
                    std::cout << "Enter scalar value: ";
                    std::cin >> scalar;
                    
                    if (idx < 0 || idx >= matrices.size()) {
                        std::cout << "Invalid matrix index." << std::endl;
                        break;
                    }
                    
                    SparseMatrix result = matrices[idx].scalarDivide(scalar);
                    matrices.push_back(result);
                    std::cout << "Result stored as matrix " << matrices.size() - 1 << std::endl;
                    result.display();
                    break;
                }
                case 7: {  // Transpose a matrix
                    if (matrices.empty()) {
                        std::cout << "No matrices available. Create a matrix first." << std::endl;
                        break;
                    }
                    
                    int idx;
                    std::cout << "Enter index of matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx;
                    
                    if (idx < 0 || idx >= matrices.size()) {
                        std::cout << "Invalid matrix index." << std::endl;
                        break;
                    }
                    
                    SparseMatrix result = matrices[idx].transpose();
                    matrices.push_back(result);
                    std::cout << "Result stored as matrix " << matrices.size() - 1 << std::endl;
                    result.display();
                    break;
                }
                case 8: {  // Calculate determinant
                    if (matrices.empty()) {
                        std::cout << "No matrices available. Create a matrix first." << std::endl;
                        break;
                    }
                    
                    int idx;
                    std::cout << "Enter index of matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx;
                    
                    if (idx < 0 || idx >= matrices.size()) {
                        std::cout << "Invalid matrix index." << std::endl;
                        break;
                    }
                    
                    double det = matrices[idx].determinant();
                    std::cout << "Determinant: " << det << std::endl;
                    break;
                }
                case 9: {  // Calculate inverse
                    if (matrices.empty()) {
                        std::cout << "No matrices available. Create a matrix first." << std::endl;
                        break;
                    }
                    
                    int idx;
                    std::cout << "Enter index of matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx;
                    
                    if (idx < 0 || idx >= matrices.size()) {
                        std::cout << "Invalid matrix index." << std::endl;
                        break;
                    }
                    
                    SparseMatrix result = matrices[idx].inverse();
                    matrices.push_back(result);
                    std::cout << "Result stored as matrix " << matrices.size() - 1 << std::endl;
                    result.display();
                    break;
                }
                case 10: {  // View matrix
                    if (matrices.empty()) {
                        std::cout << "No matrices available. Create a matrix first." << std::endl;
                        break;
                    }
                    
                    int idx;
                    std::cout << "Enter index of matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx;
                    
                    if (idx < 0 || idx >= matrices.size()) {
                        std::cout << "Invalid matrix index." << std::endl;
                        break;
                    }
                    
                    matrices[idx].display();
                    break;
                }
                case 11: {  // View sparse representation
                    if (matrices.empty()) {
                        std::cout << "No matrices available. Create a matrix first." << std::endl;
                        break;
                    }
                    
                    int idx;
                    std::cout << "Enter index of matrix (0-" << matrices.size() - 1 << "): ";
                    std::cin >> idx;
                    
                    if (idx < 0 || idx >= matrices.size()) {
                        std::cout << "Invalid matrix index." << std::endl;
                        break;
                    }
                    
                    matrices[idx].displaySparse();
                    break;
                }
                case 12: {  // Run tests
                    runTests();
                    break;
                }
                case 0: {  // Exit
                    std::cout << "Exiting program." << std::endl;
                    break;
                }
                default: {
                    std::cout << "Invalid choice. Please try again." << std::endl;
                    break;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}