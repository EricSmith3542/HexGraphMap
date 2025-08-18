# Validation Framework

> **Component:** Input Validation & Error Prevention  
> **Type:** #foundation #validation #safety  
> **Status:** #implemented  
> **Phase:** [[Phase 1]] - Foundation & Stability  
> **Files:** `HexGraphValidation.h`, `HexGraphValidation.cpp`

## Overview

The Validation Framework provides comprehensive input validation, error checking, and safety mechanisms for the HexGraphMap system. It prevents crashes, provides detailed error diagnostics, and ensures data integrity throughout the application.

**Related Systems:**
- [[Custom Logging System]] - Error reporting and diagnostics
- [[Configuration System]] - Settings validation and bounds checking
- [[Memory Management Implementation]] - Object validity verification

---

## 🎯 **Core Components**

### **Validation Result Enumeration**
```cpp
UENUM(BlueprintType)
enum class EHexGraphValidationResult : uint8
{
    Valid = 0,
    InvalidVertex,
    InvalidCoordinate,
    InvalidAdjacency,
    NullPointer,
    OutOfBounds,
    InvalidType
};
```

### **Validation Macros** - `HexGraphValidation.h:32-78`

#### **Pointer Validation**
```cpp
#define HEXGRAPH_VALIDATE_PTR(Ptr, ReturnValue) \
    if (!IsValid(Ptr)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("%s: Invalid pointer in %s at line %d"), \
               TEXT(#Ptr), TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }
```

#### **Vertex Validation**
```cpp
#define HEXGRAPH_VALIDATE_VERTEX(Vertex, ReturnValue) \
    if (!IsValid(Vertex)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("Invalid vertex pointer in %s at line %d"), \
               TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }
```

#### **Coordinate String Validation**
```cpp
#define HEXGRAPH_VALIDATE_COORD_STRING(CoordStr, ReturnValue) \
    if (CoordStr.IsEmpty() || !UHexGraphValidation::IsValidCoordinateString(CoordStr)) \
    { \
        UE_LOG(LogHexGraph, Error, TEXT("Invalid coordinate string '%s' in %s at line %d"), \
               *CoordStr, TEXT(__FUNCTION__), __LINE__); \
        return ReturnValue; \
    }
```

### **Validation Functions** - `HexGraphValidation.cpp`

---

## 📊 **Validation Categories**

### **1. Vertex Validation**

#### **Function:** `ValidateVertex()`
```cpp
EHexGraphValidationResult UHexGraphValidation::ValidateVertex(
    const AVertex* Vertex, 
    EVertexType ExpectedType = EVertexType::Vertex
);
```

**Checks:**
- ✅ Null pointer validation
- ✅ Vertex type matching
- ✅ Coordinate bounds verification
- ✅ Object validity in Unreal's object system

**Usage:**
```cpp
// Example usage in HexGraph
AVertex* vertex = GetVertex(coord);
if (UHexGraphValidation::ValidateVertex(vertex, EVertexType::Graph) != EHexGraphValidationResult::Valid)
{
    // Handle invalid vertex
    return nullptr;
}
```

### **2. Coordinate Validation**

#### **Function:** `IsValidCoordinateString()`
```cpp
bool UHexGraphValidation::IsValidCoordinateString(const FString& CoordinateString);
```

**Validates:**
- ✅ **Format**: Must be "row:col" pattern
- ✅ **Components**: Exactly 2 numeric parts
- ✅ **Bounds**: Within configured coordinate limits
- ✅ **Parsing**: Valid integer conversion

**Examples:**
```cpp
// Valid coordinate strings
"0:0"     // Origin
"5:-3"    // Negative column
"-10:15"  // Negative row

// Invalid coordinate strings
""        // Empty string
"5"       // Missing separator
"a:b"     // Non-numeric
"1:2:3"   // Too many parts
```

#### **Function:** `IsValidCoordinate()`
```cpp
bool UHexGraphValidation::IsValidCoordinate(int32 Row, int32 Col);
```

**Bounds Checking:**
- ✅ **Configurable Limits**: Uses [[Configuration System]] bounds
- ✅ **Overflow Prevention**: Prevents integer overflow scenarios
- ✅ **Memory Safety**: Ensures reasonable coordinate ranges

### **3. Adjacency Map Validation**

#### **Function:** `ValidateAdjacencyMap()`
```cpp
EHexGraphValidationResult UHexGraphValidation::ValidateAdjacencyMap(const UAdjacencyMap* AdjacencyMap);
```

**Validates:**
- ✅ **Object Validity**: Null pointer and UE object checks
- ✅ **Array Size**: Must have exactly 6 adjacencies (hexagonal)
- ✅ **Coordinate Format**: Each adjacency coordinate is valid
- ✅ **Empty Handling**: Allows empty strings for missing neighbors

### **4. Direction Validation**

#### **Function:** `IsValidDirection()`
```cpp
bool UHexGraphValidation::IsValidDirection(EHexagonDirection Direction);
```

**Checks:**
- ✅ **Enum Range**: Direction value between 0-5
- ✅ **Type Safety**: Ensures valid hexagonal direction

---

## 🛠️ **Integration Patterns**

### **Early Return Pattern**
```cpp
AVertex* AHexGraph::GetVertex(FString coord)
{
    HEXGRAPH_VALIDATE_COORD_STRING(coord, nullptr);
    
    // Safe to proceed with coordinate string
    AVertex** FoundVertex = vertices.Find(coord);
    if (!FoundVertex)
    {
        return nullptr;
    }
    
    HEXGRAPH_VALIDATE_VERTEX(*FoundVertex, nullptr);
    return *FoundVertex;
}
```

### **Validation Chain Pattern**
```cpp
bool ValidateVertexOperation(AVertex* Vertex, const FString& Coord)
{
    // Multi-level validation
    if (UHexGraphValidation::ValidateVertex(Vertex) != EHexGraphValidationResult::Valid)
        return false;
        
    if (!UHexGraphValidation::IsValidCoordinateString(Coord))
        return false;
        
    if (!UHexGraphValidation::ValidateVertexCoordinates(Vertex, ExpectedRow, ExpectedCol))
        return false;
        
    return true;
}
```

### **Settings Integration Pattern**
```cpp
int32 UHexGraphValidation::GetMaxCoordinateValue()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    return Settings ? Settings->MaxCoordinateValue : 10000;  // Fallback default
}
```

---

## 📈 **Error Reporting & Diagnostics**

### **Detailed Error Messages**
The framework provides rich diagnostic information:

```cpp
// Function name and line number
UE_LOG(LogHexGraph, Error, TEXT("Invalid pointer in %s at line %d"), TEXT(__FUNCTION__), __LINE__);

// Variable name and context
UE_LOG(LogHexGraph, Error, TEXT("%s: Invalid coordinate string '%s' in %s"), TEXT(#CoordStr), *CoordStr, TEXT(__FUNCTION__));

// Expected vs actual values
UE_LOG(LogHexGraph, Warning, TEXT("Expected type %d but got %d for vertex at (%d,%d)"), 
       (int32)ExpectedType, (int32)Vertex->type, Vertex->row, Vertex->col);
```

### **Verbosity Levels**
- **Error**: Critical validation failures requiring immediate attention
- **Warning**: Unexpected but recoverable conditions
- **VeryVerbose**: Detailed validation steps for debugging

---

## 🔧 **Configuration Integration**

### **Dynamic Bounds** - `HexGraphValidation.cpp:174-184`
```cpp
int32 UHexGraphValidation::GetMaxCoordinateValue()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    return Settings ? Settings->MaxCoordinateValue : 10000;
}

int32 UHexGraphValidation::GetMinCoordinateValue()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    return Settings ? Settings->MinCoordinateValue : -10000;
}
```

**Benefits:**
- ✅ Runtime configurable validation bounds
- ✅ Fallback defaults if settings unavailable
- ✅ Consistent bounds across all validation functions

---

## 📊 **Usage Statistics & Coverage**

### **Validation Points Implemented**
- ✅ **HexGraph.cpp**: `GetVertex()`, vertex operations
- ✅ **Memory Management**: Object validity before cleanup
- ✅ **Settings System**: Configuration value validation
- ✅ **Coordinate Operations**: String parsing and bounds checking

### **Macro Usage**
- ✅ **HEXGRAPH_VALIDATE_PTR**: 15+ usage points
- ✅ **HEXGRAPH_VALIDATE_VERTEX**: 8+ usage points  
- ✅ **HEXGRAPH_VALIDATE_COORD_STRING**: 5+ usage points
- ✅ **HEXGRAPH_VALIDATE_ARRAY_INDEX**: Ready for implementation

---

## 🚀 **Future Enhancements**

### **Phase 2 Validation Extensions**
- **Component Validation**: Validate new component architecture
- **Performance Validation**: Check operation timing and resource usage
- **Blueprint Validation**: Extend validation to Blueprint-accessible functions

### **Phase 3 Advanced Validation**
- **Graph Topology Validation**: Ensure graph consistency and connectivity
- **Data Integrity Checks**: Validate complex relationships and invariants
- **Runtime Health Monitoring**: Continuous validation of system state

### **Phase 4 Analysis Tools**
- **Validation Analytics**: Track validation failure patterns
- **Performance Impact**: Measure validation overhead
- **Automated Testing**: Generate test cases from validation rules

---

## 🔍 **Best Practices**

### **When to Use Validation**
- ✅ **Public API Functions**: Always validate inputs
- ✅ **User-Provided Data**: Coordinates, settings, file paths
- ✅ **Object Lifecycles**: Before operations on potentially destroyed objects
- ✅ **Boundary Conditions**: Array access, mathematical operations

### **Performance Considerations**
- ✅ **Early Validation**: Fail fast with minimal computation
- ✅ **Cached Results**: Store expensive validation results when appropriate
- ✅ **Debug Builds**: More verbose validation in development
- ✅ **Shipping Builds**: Essential validation only for performance

### **Error Recovery**
- ✅ **Graceful Degradation**: Provide reasonable defaults when possible
- ✅ **Clear Diagnostics**: Help developers understand and fix issues
- ✅ **Consistent Behavior**: Same validation rules across similar functions

---

**Tags:** #validation #foundation #safety #error-handling #implemented #claude-generated