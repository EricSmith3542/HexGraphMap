# Settings Migration

> **Component:** Hard-coded Value Migration  
> **Type:** #configuration #migration #refactoring  
> **Status:** #implemented  
> **Phase:** [[Phase 1]] - Foundation & Stability  
> **Parent:** [[Configuration System]]

## Overview

Settings Migration documents the process of moving hard-coded values from throughout the HexGraphMap codebase into the centralized [[Configuration System]]. This migration eliminates magic numbers and provides runtime configurability.

**Related Systems:**
- [[Configuration System]] - Target settings architecture
- [[HexGraphSettings]] - Settings class implementation
- [[Runtime Configuration]] - Dynamic settings updates

---

## 🎯 **Migration Summary**

### **Values Migrated to Settings**

#### **Graph Configuration**
```cpp
// Before: Hard-coded in HexGraph.cpp
maxFillDepth = 5;                    // Line 43
VertexSpacing = 0.5f;               // Line 26
MeshLength = -1;                    // Line 25

// After: Centralized in HexGraphSettings
Settings->MaxFillDepth              // Default: 5
Settings->DefaultVertexSpacing      // Default: 100.0f  
Settings->DefaultMeshLength         // Default: 100.0f
```

#### **Camera Configuration**
```cpp
// Before: Hard-coded in HexGraph.h
PanSpeed = 15.f;                    // Line 262
ZoomPercent = 100.f;                // Line 261

// After: Centralized in HexGraphSettings
Settings->DefaultPanSpeed           // Default: 15.0f
Settings->DefaultZoomPercent        // Default: 100.0f
```

#### **Validation Configuration**
```cpp
// Before: Hard-coded in HexGraphValidation.h
MAX_COORDINATE_VALUE = 10000;       // Line 143
MIN_COORDINATE_VALUE = -10000;      // Line 144

// After: Dynamic from settings
Settings->MaxCoordinateValue        // Default: 10000
Settings->MinCoordinateValue        // Default: -10000
```

#### **Debug Configuration**
```cpp
// Before: Hard-coded flags
useMouseFollower = false;           // Implicit default

// After: Configurable setting
Settings->bUseMouseFollower         // Default: false
```

---

## 🔄 **Migration Process**

### **1. Identify Hard-coded Values**
Search patterns used to find values to migrate:
- Magic numbers in constructors
- Constant definitions in headers
- Hard-coded limits and bounds
- Boolean flags and switches

### **2. Add to Settings Class**
```cpp
// Add UPROPERTY to HexGraphSettings.h
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Graph", 
          meta = (ClampMin = "1", ClampMax = "100"))
int32 MaxFillDepth = 5;
```

### **3. Update Constructor Initialization**
```cpp
// Replace direct assignment
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
maxFillDepth = Settings->MaxFillDepth;
VertexSpacing = Settings->DefaultVertexSpacing;
```

### **4. Update Runtime Access**
```cpp
// Replace magic numbers with settings access
if (depth <= Settings->MaxFillDepth)  // Instead of: depth <= 5
{
    // Fill operation
}
```

---

## 📊 **Migration Benefits**

### **Before Migration**
❌ **Magic Numbers**: Hard-coded values scattered throughout code  
❌ **No Runtime Changes**: Required recompilation to adjust parameters  
❌ **Team Conflicts**: Different developers using different values  
❌ **Hard to Find**: Values buried in implementation files  

### **After Migration**
✅ **Centralized Values**: All configuration in one settings class  
✅ **Runtime Configurable**: Adjust values in Project Settings panel  
✅ **Team Consistency**: Shared configuration files in version control  
✅ **Self-Documenting**: Settings with categories and validation  

---

## 🛠️ **Validation Integration**

### **Dynamic Bounds** - `HexGraphValidation.cpp:174-184`
```cpp
int32 UHexGraphValidation::GetMaxCoordinateValue()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    return Settings ? Settings->MaxCoordinateValue : 10000;  // Fallback default
}

int32 UHexGraphValidation::GetMinCoordinateValue()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    return Settings ? Settings->MinCoordinateValue : -10000;  // Fallback default
}
```

**Benefits:**
- ✅ **Runtime Validation**: Validation bounds configurable at runtime
- ✅ **Fallback Safety**: Safe defaults if settings unavailable
- ✅ **Consistent Bounds**: Same limits used throughout validation system

---

## 📈 **Configuration Categories**

### **Graph Settings**
- `DefaultVertexSpacing`: Distance between adjacent vertices
- `MaxFillDepth`: Maximum recursion depth for fill operations  
- `DefaultMeshLength`: Base size for vertex mesh components

### **Camera Settings**
- `DefaultPanSpeed`: Camera movement sensitivity
- `DefaultZoomPercent`: Initial camera zoom level

### **Memory Settings**
- `ExpectedMaxVertices`: Expected vertex count for optimization
- `ExpectedMaxAdjacencyMaps`: Expected adjacency map count
- `bAutoCleanupEnabled`: Enable automatic memory cleanup

### **Validation Settings**
- `MaxCoordinateValue`: Upper bound for coordinate values
- `MinCoordinateValue`: Lower bound for coordinate values
- `bStrictValidationEnabled`: Enable detailed validation checking

### **Debug Settings**
- `bUseMouseFollower`: Enable mouse following debug visual
- `bEnableVerboseLogging`: Enable detailed log output
- `bShowValidationWarnings`: Show/hide validation messages

---

## 🚀 **Future Migration Opportunities**

### **Phase 2 Candidates**
- Component-specific configuration values
- Performance tuning parameters
- Input system configuration

### **Phase 3 Extensions**
- User preference settings
- Gameplay balance parameters
- UI layout and appearance settings

---

For complete implementation details, see: [[Configuration System]]

**Tags:** #settings-migration #configuration #refactoring #hard-coded-values #implemented #claude-generated