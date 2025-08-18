# Managed Object System

> **Component:** Object Lifecycle Management  
> **Type:** #foundation #memory #object-tracking  
> **Status:** #implemented  
> **Phase:** [[Phase 1]] - Foundation & Stability  
> **Parent:** [[Memory Management Implementation]]

## Overview

The Managed Object System provides controlled creation, tracking, and cleanup of `UAdjacencyMap` objects in the HexGraphMap project. This system prevents memory leaks by ensuring all created objects are properly tracked and cleaned up during object destruction.

**Related Systems:**
- [[Memory Management Implementation]] - Overall memory management strategy
- [[Custom Logging System]] - Object lifecycle logging and diagnostics
- [[Validation Framework]] - Object validity verification

---

## 🎯 **Core Architecture**

### **Object Tracking Array** - `HexGraph.h:61-63`
```cpp
// Managed adjacency map tracking for proper cleanup
UPROPERTY()
TArray<TObjectPtr<UAdjacencyMap>> ManagedAdjacencyMaps;
```

**Design Features:**
- ✅ **UPROPERTY()**: Ensures garbage collector visibility
- ✅ **TObjectPtr<>**: Safe object references with automatic null handling
- ✅ **TArray**: Dynamic collection for unlimited object tracking
- ✅ **Private Ownership**: Objects created with HexGraph as outer

### **Managed Creation Interface** - `HexGraph.h:125-130`
```cpp
// Managed adjacency map system
UFUNCTION(BlueprintCallable, Category = "HexGraph")
UAdjacencyMap* CreateManagedAdjacencyMap();

UFUNCTION(BlueprintCallable, Category = "HexGraph")
void CleanupManagedAdjacencyMaps();
```

---

## 🔧 **Implementation Details**

### **1. Controlled Object Creation**

#### **CreateManagedAdjacencyMap()** - `HexGraph.cpp:358-365`
```cpp
UAdjacencyMap* AHexGraph::CreateManagedAdjacencyMap()
{
    HEXGRAPH_VALIDATE_PTR(this, nullptr);
    
    UAdjacencyMap* NewAdjMap = NewObject<UAdjacencyMap>(this);
    if (IsValid(NewAdjMap))
    {
        ManagedAdjacencyMaps.Add(NewAdjMap);
        UE_LOG(LogHexGraph, VeryVerbose, TEXT("CreateManagedAdjacencyMap: Created and tracked new adjacency map"));
    }
    else
    {
        UE_LOG(LogHexGraph, Error, TEXT("CreateManagedAdjacencyMap: Failed to create adjacency map"));
    }
    
    return NewAdjMap;
}
```

**Creation Process:**
1. **Validation**: Ensures `this` pointer is valid before proceeding
2. **Object Creation**: Uses `NewObject<>()` with proper outer object
3. **Validity Check**: Verifies successful creation before tracking
4. **Tracking Registration**: Adds object to managed collection
5. **Logging**: Records creation for debugging and monitoring

**Benefits:**
- ✅ **Single Point of Creation**: All adjacency maps created through managed system
- ✅ **Automatic Tracking**: No manual tracking required
- ✅ **Error Handling**: Graceful failure handling with logging
- ✅ **Blueprint Access**: Available to Blueprint system for flexibility

### **2. Object Lifecycle Management**

#### **Integration with InitializeVertex()** - `HexGraph.cpp:655-700`
```cpp
void AHexGraph::InitializeVertex(AVertex* vertex, int row, int col, bool isTemp, UAdjacencyMap* adjacencyMap)
{
    // ... vertex initialization code ...
    
    // Create or use provided adjacency map
    UAdjacencyMap* vertexAdjacencyMap;
    if (adjacencyMap)
    {
        vertexAdjacencyMap = adjacencyMap;
    }
    else
    {
        // Use managed creation for new adjacency maps
        vertexAdjacencyMap = CreateManagedAdjacencyMap();
    }
    
    // Store in appropriate collection
    if (isTemp)
    {
        tempAdjacencyMatrix[vertex->Coord()] = vertexAdjacencyMap;
    }
    else
    {
        adjacencyMatrix[vertex->Coord()] = vertexAdjacencyMap;
    }
    
    // ... rest of initialization ...
}
```

**Integration Features:**
- ✅ **Conditional Creation**: Only creates if not provided
- ✅ **Managed by Default**: All new objects are automatically tracked
- ✅ **Backward Compatibility**: Accepts pre-existing adjacency maps
- ✅ **Flexible Storage**: Works with both temp and permanent collections

### **3. Cleanup and Validation**

#### **CleanupManagedAdjacencyMaps()** - `HexGraph.cpp:367-384`
```cpp
void AHexGraph::CleanupManagedAdjacencyMaps()
{
    UE_LOG(LogHexGraph, Log, TEXT("CleanupManagedAdjacencyMaps: Starting cleanup of %d managed adjacency maps"), 
           ManagedAdjacencyMaps.Num());
    
    int32 CleanedObjects = 0;
    
    // Reverse iteration for safe removal
    for (int32 i = ManagedAdjacencyMaps.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(ManagedAdjacencyMaps[i]))
        {
            ManagedAdjacencyMaps.RemoveAt(i);
            CleanedObjects++;
        }
    }
    
    UE_LOG(LogHexGraph, Log, TEXT("CleanupManagedAdjacencyMaps: Cleaned up %d invalid objects, %d remaining"), 
           CleanedObjects, ManagedAdjacencyMaps.Num());
}
```

**Cleanup Algorithm:**
1. **Statistics Logging**: Reports cleanup start and object count
2. **Reverse Iteration**: Safe removal during array traversal
3. **Validity Check**: Only removes actually invalid objects
4. **Safe Removal**: Uses `RemoveAt()` to prevent array corruption
5. **Results Reporting**: Logs cleanup effectiveness

**Safety Features:**
- ✅ **Non-Destructive**: Only removes invalid references, not valid objects
- ✅ **Iterator Safety**: Reverse iteration prevents index corruption
- ✅ **Statistics Tracking**: Monitors cleanup effectiveness
- ✅ **Garbage Collection Integration**: Works with UE's GC system

---

## 🔄 **Object Lifecycle**

### **Creation Lifecycle**
```mermaid
graph TD
    A[Vertex Needs Adjacency Map] --> B{Adjacency Map Provided?}
    B -->|No| C[CreateManagedAdjacencyMap()]
    B -->|Yes| D[Use Provided Map]
    C --> E[NewObject<UAdjacencyMap>()]
    E --> F[Add to ManagedAdjacencyMaps]
    F --> G[Return to Caller]
    D --> G
    G --> H[Store in Collection]
```

### **Cleanup Lifecycle**
```mermaid
graph TD
    A[EndPlay() Called] --> B[CleanupManagedAdjacencyMaps()]
    B --> C[Iterate Through Tracked Objects]
    C --> D{Object Valid?}
    D -->|No| E[Remove from Tracking Array]
    D -->|Yes| F[Keep in Array]
    E --> G[Continue Iteration]
    F --> G
    G --> H{More Objects?}
    H -->|Yes| C
    H -->|No| I[Log Cleanup Results]
```

---

## 📊 **Memory Management Benefits**

### **Before Managed System**
❌ **Untracked Objects**: `UAdjacencyMap` objects created without tracking  
❌ **Memory Leaks**: Objects not cleaned up when HexGraph destroyed  
❌ **Manual Management**: Developers responsible for remembering cleanup  
❌ **Inconsistent Patterns**: Different creation patterns throughout code  

### **After Managed System**
✅ **Automatic Tracking**: All objects tracked from creation  
✅ **Guaranteed Cleanup**: Cleanup integrated into object lifecycle  
✅ **Consistent Interface**: Single creation method for all adjacency maps  
✅ **Error Prevention**: Validation and logging prevent common mistakes  

---

## 📈 **Performance Characteristics**

### **Memory Overhead**
- **Tracking Array**: Minimal overhead for `TArray<TObjectPtr<>>`
- **Per Object**: No additional memory per tracked object
- **GC Integration**: Leverages existing garbage collection system

### **Runtime Performance**
- **Creation**: Single array insertion per object
- **Cleanup**: O(n) iteration where n = tracked object count
- **Access**: No performance impact on normal object usage

### **Optimization Opportunities**
```cpp
// Future: Batch cleanup for better performance
void CleanupManagedAdjacencyMaps(bool bForceFullCleanup = false)
{
    if (!bForceFullCleanup && ManagedAdjacencyMaps.Num() < CleanupThreshold)
    {
        return; // Skip cleanup if below threshold
    }
    
    // ... existing cleanup logic ...
}
```

---

## 🛠️ **Configuration Integration**

### **Memory Settings** - `HexGraphSettings.h:39-49`
```cpp
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory")
int32 ExpectedMaxAdjacencyMaps = 10000;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory")
bool bAutoCleanupEnabled = true;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory")
float CleanupIntervalSeconds = 10.0f;
```

### **Future Configuration Usage**
```cpp
// Reserve capacity based on expected usage
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
ManagedAdjacencyMaps.Reserve(Settings->ExpectedMaxAdjacencyMaps);

// Conditional cleanup based on settings
if (Settings->bAutoCleanupEnabled)
{
    CleanupManagedAdjacencyMaps();
}
```

---

## 🔍 **Debugging and Monitoring**

### **Creation Monitoring**
```cpp
UE_LOG(LogHexGraph, VeryVerbose, TEXT("CreateManagedAdjacencyMap: Created and tracked new adjacency map (Total: %d)"), 
       ManagedAdjacencyMaps.Num());
```

### **Cleanup Statistics**
```cpp
UE_LOG(LogHexGraph, Log, TEXT("CleanupManagedAdjacencyMaps: Cleaned up %d invalid objects, %d remaining"), 
       CleanedObjects, ManagedAdjacencyMaps.Num());
```

### **Memory Health Checks**
```cpp
// Future: Memory health reporting
void ReportMemoryHealth()
{
    int32 ValidObjects = 0;
    for (const auto& AdjMap : ManagedAdjacencyMaps)
    {
        if (IsValid(AdjMap))
            ValidObjects++;
    }
    
    float HealthPercentage = (float)ValidObjects / ManagedAdjacencyMaps.Num() * 100.0f;
    UE_LOG(LogHexGraph, Log, TEXT("Memory Health: %.1f%% (%d/%d objects valid)"), 
           HealthPercentage, ValidObjects, ManagedAdjacencyMaps.Num());
}
```

---

## 🚀 **Future Enhancements**

### **Phase 2 Extensions**
- **Generic Managed Objects**: Extend system to track other object types
- **Object Pooling**: Reuse adjacency maps to reduce allocation pressure
- **Memory Budgets**: Enforce memory limits and trigger cleanup

### **Phase 3 Advanced Features**
- **Reference Counting**: Track usage of shared adjacency maps
- **Weak References**: Non-owning references for complex relationships
- **Memory Analytics**: Detailed tracking of allocation patterns

### **Phase 4 Production Features**
- **Memory Profiling**: Integration with Unreal's memory profiler
- **Automated Optimization**: Self-tuning memory management parameters
- **Remote Monitoring**: Network-based memory usage monitoring

---

**Tags:** #managed-objects #memory #lifecycle #object-tracking #implemented #claude-generated