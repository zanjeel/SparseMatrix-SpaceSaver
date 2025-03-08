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
};

int main() {
    try {
        // Test 1: Create a sparse matrix
        std::cout << "Creating a 3x3 sparse matrix..." << std::endl;
        SparseMatrix m1(3, 3);
        m1.insert(0, 0, 1.0);
        m1.insert(0, 2, 2.0);
        m1.insert(2, 1, 3.0);
        
        std::cout << "\nOriginal Matrix:" << std::endl;
        m1.display();
        
        std::cout << "\nSparse Representation:" << std::endl;
        m1.displaySparse();
        
        // Test 2: Create another matrix for operations
        std::cout << "\nCreating second 3x3 matrix..." << std::endl;
        SparseMatrix m2(3, 3);
        m2.insert(0, 0, 4.0);
        m2.insert(1, 1, 5.0);
        m2.insert(2, 2, 6.0);
        
        std::cout << "\nSecond Matrix:" << std::endl;
        m2.display();
        
        // Test 3: Addition
        std::cout << "\nTesting Addition:" << std::endl;
        SparseMatrix sum = m1.add(m2);
        sum.display();
        
        // Test 4: Transpose
        std::cout << "\nTesting Transpose of first matrix:" << std::endl;
        SparseMatrix trans = m1.transpose();
        trans.display();
        
        // Test 5: Scalar Multiplication
        std::cout << "\nTesting Scalar Multiplication (first matrix * 2):" << std::endl;
        SparseMatrix scaled = m1.scalarMultiply(2.0);
        scaled.display();
        
        // Test 6: Matrix Multiplication
        std::cout << "\nTesting Matrix Multiplication:" << std::endl;
        SparseMatrix product = m1.multiply(m2);
        product.display();
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 