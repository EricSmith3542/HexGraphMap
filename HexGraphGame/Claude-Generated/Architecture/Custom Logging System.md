# Custom Logging System

> **Component:** Custom Logging  
> **Type:** #foundation #logging #debugging  
> **Status:** #implemented  
> **Phase:** [[Phase 1]] - Foundation & Stability  
> **Files:** `HexGraphMap.h`, `HexGraphMap.cpp`

## Overview

The Custom Logging System replaces Unreal Engine's default `LogTemp` category with a dedicated `LogHexGraph` logging category specifically for the HexGraphMap project. This provides better organization, filtering, and debugging capabilities.

**Related Systems:**
- [[Validation Framework]] - Uses LogHexGraph for error reporting
- [[Memory Management Implementation]] - Logs cleanup operations
- [[Configuration System]] - Logs settings changes

---

## 🎯 **Implementation Details**

### **Declaration** - `HexGraphMap.h:9`
```cpp
DECLARE_LOG_CATEGORY_EXTERN(LogHexGraph, Log, All);
```

### **Definition** - `HexGraphMap.cpp:5`
```cpp
DEFINE_LOG_CATEGORY(LogHexGraph);
```

### **Usage Pattern**
```cpp
// Before (old pattern)
UE_LOG(LogTemp, Warning, TEXT("Some debug message"));

// After (new pattern)
UE_LOG(LogHexGraph, Warning, TEXT("Some debug message"));
```

---

## 📊 **Log Categories & Usage**

### **Error Level** - Critical Issues
```cpp
UE_LOG(LogHexGraph, Error, TEXT("Invalid pointer in %s at line %d"), TEXT(__FUNCTION__), __LINE__);
```
**Usage:** Validation failures, null pointer access, critical system failures

### **Warning Level** - Potential Issues
```cpp
UE_LOG(LogHexGraph, Warning, TEXT("Expected type %d but got %d for vertex"), ExpectedType, ActualType);
```
**Usage:** Type mismatches, unexpected conditions, deprecated usage

### **Log Level** - General Information
```cpp
UE_LOG(LogHexGraph, Log, TEXT("EndPlay: HexGraph cleanup completed"));
```
**Usage:** System lifecycle events, configuration changes, general operations

### **Verbose/VeryVerbose Level** - Detailed Debug Info
```cpp
UE_LOG(LogHexGraph, VeryVerbose, TEXT("Coordinate validation: (%d, %d) bounds [%d, %d]"), Row, Col, Min, Max);
```
**Usage:** Detailed validation steps, coordinate calculations, performance metrics

---

## 🔧 **Integration Points**

### **Validation Framework Integration**
- All validation macros use `LogHexGraph` for error reporting
- Provides consistent error formatting across validation functions
- Enables easy filtering of validation-related messages

### **Memory Management Integration**
- Logs cleanup operations and memory statistics
- Tracks managed object lifecycle events
- Reports memory leak detection results

### **Settings System Integration**
- Logs configuration changes and validation results
- Reports settings loading and parsing status
- Tracks runtime configuration updates

---

## 🛠️ **Configuration & Filtering**

### **Editor Output Log Filtering**
1. Open **Window** → **Developer Tools** → **Output Log**
2. Use the **Category** dropdown to select `LogHexGraph`
3. Set **Verbosity** level as needed (Error, Warning, Log, Verbose, VeryVerbose)

### **Runtime Log Level Control**
```cpp
// Set via console command
LogHexGraph Verbose

// Set via C++ code
LogHexGraph.SetVerbosity(ELogVerbosity::Verbose);
```

### **Build Configuration**
- **Development/Debug Builds**: All log levels available
- **Shipping Builds**: Error/Warning only (configurable via build settings)
- **Test Builds**: Log level and above

---

## 📈 **Benefits & Advantages**

### **Organization**
- **Dedicated Category**: Clear separation from engine logs and other project logs
- **Consistent Naming**: All HexGraph-related logs use the same category
- **Easy Filtering**: Can view only HexGraph logs in Output Log window

### **Debugging**
- **Contextual Information**: Function names, line numbers, and call context
- **Structured Messages**: Consistent formatting across different systems
- **Performance Tracking**: Detailed timing and operation counts when needed

### **Maintenance**
- **Searchable Logs**: Easy to find specific log messages in large log files
- **Level Control**: Can adjust verbosity without code changes
- **Team Collaboration**: Shared understanding of log message meanings

---

## 🔍 **Common Usage Patterns**

### **Function Entry/Exit Logging**
```cpp
void AHexGraph::SomeFunction()
{
    UE_LOG(LogHexGraph, VeryVerbose, TEXT("SomeFunction: Entry"));
    
    // Function implementation
    
    UE_LOG(LogHexGraph, VeryVerbose, TEXT("SomeFunction: Exit"));
}
```

### **Error Condition Reporting**
```cpp
if (!IsValid(Vertex))
{
    UE_LOG(LogHexGraph, Error, TEXT("Invalid vertex pointer in %s at line %d"), 
           TEXT(__FUNCTION__), __LINE__);
    return nullptr;
}
```

### **Performance Metrics**
```cpp
double StartTime = FPlatformTime::Seconds();
// ... operation ...
double ElapsedTime = FPlatformTime::Seconds() - StartTime;
UE_LOG(LogHexGraph, Log, TEXT("Operation completed in %f seconds"), ElapsedTime);
```

### **State Change Logging**
```cpp
UE_LOG(LogHexGraph, Log, TEXT("Settings updated - VertexSpacing: %f, MaxFillDepth: %d"), 
       VertexSpacing, maxFillDepth);
```

---

## 📋 **Migration Status**

### **Completed Migrations**
- ✅ `HexGraph.cpp` - All `LogTemp` usage converted to `LogHexGraph`
- ✅ `HexGraphValidation.cpp` - All validation messages use `LogHexGraph`
- ✅ `HexGraphSettings.cpp` - Configuration logging standardized
- ✅ Validation macros - Error reporting uses `LogHexGraph`

### **Log Message Categories**
- ✅ **Memory Management**: Object creation, cleanup, leak detection
- ✅ **Validation**: Input validation, type checking, bounds verification
- ✅ **Configuration**: Settings loading, validation, runtime changes
- ✅ **Lifecycle**: Object construction, destruction, state changes

---

## 🚀 **Future Enhancements**

### **Structured Logging** (Phase 2)
- JSON-formatted log messages for automated parsing
- Correlation IDs for tracking operations across multiple functions
- Performance counters and memory usage metrics

### **Remote Logging** (Phase 3)
- Network logging for multiplayer debugging
- Centralized log collection for team development
- Real-time log streaming and filtering

### **Analytics Integration** (Phase 4)
- Automatic error reporting and analytics
- Performance baseline tracking
- User behavior analytics for UX improvements

---

**Tags:** #logging #foundation #debugging #implemented #claude-generated