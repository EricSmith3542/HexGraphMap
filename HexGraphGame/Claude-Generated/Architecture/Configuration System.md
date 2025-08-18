# Configuration System

> **Component:** Centralized Settings Management  
> **Type:** #foundation #configuration #settings  
> **Status:** #implemented  
> **Phase:** [[Phase 1]] - Foundation & Stability  
> **Files:** `HexGraphSettings.h`, `HexGraphSettings.cpp`, `HexGraphMap.Build.cs`

## Overview

The Configuration System provides centralized, runtime-configurable settings management for the HexGraphMap project. Built on Unreal Engine's `UDeveloperSettings` framework, it replaces hard-coded values with a flexible, validated configuration system accessible through the Project Settings panel.

**Related Systems:**
- [[Custom Logging System]] - Configuration change logging and validation reporting
- [[Validation Framework]] - Settings bounds checking and value validation
- [[Memory Management Implementation]] - Memory limits and cleanup configuration

---

## 🎯 **Architecture Overview**

### **Settings Class Declaration** - `HexGraphSettings.h:12`
```cpp
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Hex Graph Settings"))
class HEXGRAPHMAP_API UHexGraphSettings : public UDeveloperSettings
```

**Key Features:**
- ✅ **UDeveloperSettings Integration**: Appears in Project Settings panel
- ✅ **Configuration Persistence**: Settings saved to `DefaultGame.ini`
- ✅ **Editor Integration**: Live editing in Unreal Editor
- ✅ **Blueprint Accessibility**: Exposed to Blueprint system

### **Module Dependencies** - `HexGraphMap.Build.cs:11`
```cpp
PublicDependencyModuleNames.AddRange(new string[] { 
    "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "DeveloperSettings" 
});
```

---

## 📊 **Configuration Categories**

### **1. Graph Configuration** - `HexGraphSettings.h:24-30`
```cpp
// Graph Configuration
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Graph", meta = (ClampMin = "0.1", ClampMax = "1000.0"))
float DefaultVertexSpacing = 100.0f;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Graph", meta = (ClampMin = "1", ClampMax = "100"))
int32 MaxFillDepth = 5;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Graph", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
float DefaultMeshLength = 100.0f;
```

**Purpose:**
- ✅ **Vertex Spacing**: Distance between adjacent hexagonal vertices
- ✅ **Fill Depth**: Maximum recursion depth for fill operations
- ✅ **Mesh Length**: Base size for vertex mesh components

### **2. Camera Configuration** - `HexGraphSettings.h:32-37`
```cpp
// Camera Configuration
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "1.0", ClampMax = "100.0"))
float DefaultPanSpeed = 15.0f;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
float DefaultZoomPercent = 100.0f;
```

**Controls:**
- ✅ **Pan Speed**: Camera movement sensitivity
- ✅ **Zoom Level**: Default camera zoom percentage

### **3. Memory Management Configuration** - `HexGraphSettings.h:39-49`
```cpp
// Memory Management Configuration
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "100", ClampMax = "100000"))
int32 ExpectedMaxVertices = 10000;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "100", ClampMax = "100000"))
int32 ExpectedMaxAdjacencyMaps = 10000;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory")
bool bAutoCleanupEnabled = true;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (ClampMin = "1.0", ClampMax = "60.0"))
float CleanupIntervalSeconds = 10.0f;
```

**Memory Controls:**
- ✅ **Object Limits**: Expected maximum counts for optimization
- ✅ **Cleanup Automation**: Enable/disable automatic cleanup
- ✅ **Cleanup Frequency**: Time interval between cleanup operations

### **4. Validation Configuration** - `HexGraphSettings.h:51-59`
```cpp
// Validation Configuration
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation", meta = (ClampMin = "-100000", ClampMax = "100000"))
int32 MaxCoordinateValue = 10000;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation", meta = (ClampMin = "-100000", ClampMax = "100000"))
int32 MinCoordinateValue = -10000;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Validation")
bool bStrictValidationEnabled = true;
```

**Validation Rules:**
- ✅ **Coordinate Bounds**: Min/max allowed coordinate values
- ✅ **Strict Mode**: Enable detailed validation checking

### **5. Debug Configuration** - `HexGraphSettings.h:61-69`
```cpp
// Debug Configuration
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
bool bUseMouseFollower = false;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
bool bEnableVerboseLogging = false;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Debug")
bool bShowValidationWarnings = true;
```

**Debug Tools:**
- ✅ **Mouse Follower**: Visual debugging tool
- ✅ **Verbose Logging**: Detailed log output
- ✅ **Validation Warnings**: Show/hide validation messages

### **6. Performance Configuration** - `HexGraphSettings.h:71-77`
```cpp
// Performance Configuration
UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Performance")
bool bEnableObjectPooling = false;

UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "10", ClampMax = "1000"))
int32 ObjectPoolInitialSize = 100;
```

**Performance Tuning:**
- ✅ **Object Pooling**: Enable object reuse for performance
- ✅ **Pool Size**: Initial allocation size for object pools

---

## 🔧 **Implementation Details**

### **Settings Access Interface** - `HexGraphSettings.h:79-90`
```cpp
/**
 * Get the singleton instance of HexGraphSettings
 * @return The settings instance
 */
UFUNCTION(BlueprintCallable, Category = "Hex Graph|Settings", CallInEditor)
static const UHexGraphSettings* GetHexGraphSettings();

/**
 * Get the settings as a mutable object for runtime changes
 * @return The mutable settings instance
 */
static UHexGraphSettings* GetMutableHexGraphSettings();
```

### **Settings Singleton Implementation** - `HexGraphSettings.cpp:38-42`
```cpp
const UHexGraphSettings* UHexGraphSettings::GetHexGraphSettings()
{
    return GetDefault<UHexGraphSettings>();
}

UHexGraphSettings* UHexGraphSettings::GetMutableHexGraphSettings()
{
    return GetMutableDefault<UHexGraphSettings>();
}
```

### **Runtime Settings Refresh** - `HexGraph.cpp:472-492`
```cpp
void AHexGraph::RefreshSettingsValues()
{
    const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
    if (!Settings)
    {
        UE_LOG(LogHexGraph, Warning, TEXT("RefreshSettingsValues: Could not get HexGraphSettings, using defaults"));
        return;
    }

    // Update runtime values from settings
    VertexSpacing = Settings->DefaultVertexSpacing;
    PanSpeed = Settings->DefaultPanSpeed;
    ZoomPercent = Settings->DefaultZoomPercent;
    maxFillDepth = Settings->MaxFillDepth;
    useMouseFollower = Settings->bUseMouseFollower;
    
    UE_LOG(LogHexGraph, Log, TEXT("RefreshSettingsValues: Updated settings - VertexSpacing: %f, PanSpeed: %f, ZoomPercent: %f, MaxFillDepth: %d"), 
           VertexSpacing, PanSpeed, ZoomPercent, maxFillDepth);
}
```

---

## 📈 **Settings Validation System**

### **Constructor Validation** - `HexGraphSettings.cpp:12-26`
```cpp
UHexGraphSettings::UHexGraphSettings()
{
    // Initialize default values (already set in header, but explicit here for clarity)
    DefaultVertexSpacing = 100.0f;
    MaxFillDepth = 5;
    DefaultMeshLength = 100.0f;
    // ... other defaults
    
    ValidateSettings();
}
```

### **Validation Implementation** - `HexGraphSettings.cpp:77-153`
```cpp
void UHexGraphSettings::ValidateSettings()
{
    // Ensure vertex spacing is positive
    if (DefaultVertexSpacing <= 0.0f)
    {
        UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: DefaultVertexSpacing must be positive, resetting to 100.0"));
        DefaultVertexSpacing = 100.0f;
    }

    // Ensure max fill depth is reasonable
    if (MaxFillDepth < 1)
    {
        UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: MaxFillDepth must be at least 1, resetting to 5"));
        MaxFillDepth = 5;
    }
    else if (MaxFillDepth > 100)
    {
        UE_LOG(LogHexGraph, Warning, TEXT("HexGraphSettings: MaxFillDepth too high (%d), clamping to 100"), MaxFillDepth);
        MaxFillDepth = 100;
    }
    
    // ... additional validation rules
}
```

**Validation Features:**
- ✅ **Range Checking**: Ensures values within reasonable bounds
- ✅ **Automatic Correction**: Invalid values reset to safe defaults
- ✅ **Detailed Logging**: Explains why values were changed
- ✅ **Startup Validation**: Runs during settings construction

### **Editor Change Validation** - `HexGraphSettings.cpp:49-58`
```cpp
#if WITH_EDITOR
void UHexGraphSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Validate settings when changed in editor
    ValidateSettings();
    
    // Log the change for debugging
    if (PropertyChangedEvent.Property)
    {
        UE_LOG(LogHexGraph, Log, TEXT("HexGraphSettings: Property '%s' changed"), *PropertyChangedEvent.Property->GetName());
    }
}
#endif
```

---

## 🔄 **Integration Patterns**

### **Constructor Integration** - `HexGraph.cpp:26-34`
```cpp
// Initialize configuration from settings
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();

MeshLength = Settings->DefaultMeshLength;
VertexSpacing = Settings->DefaultVertexSpacing;
PanSpeed = Settings->DefaultPanSpeed;
ZoomPercent = Settings->DefaultZoomPercent;
maxFillDepth = Settings->MaxFillDepth;
```

**Pattern Benefits:**
- ✅ **Initialization Time**: Settings loaded once during construction
- ✅ **Default Fallbacks**: Safe defaults if settings unavailable
- ✅ **Immediate Availability**: Settings ready for first frame

### **Runtime Access Pattern**
```cpp
// Safe settings access with fallback
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
float spacing = Settings ? Settings->DefaultVertexSpacing : 100.0f;
```

### **Validation Integration Pattern** - `HexGraphValidation.cpp:174-184`
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

---

## 🎮 **User Experience**

### **Project Settings Access**
1. **Open Editor**: Launch Unreal Engine Editor
2. **Project Settings**: Edit → Project Settings
3. **Navigate**: Plugins → Hex Graph
4. **Configure**: Adjust values in real-time
5. **Apply**: Changes saved automatically to `DefaultGame.ini`

### **Settings Categories**
- 🎯 **Graph**: Core gameplay parameters
- 📹 **Camera**: View and navigation controls  
- 💾 **Memory**: Resource management settings
- ✅ **Validation**: Data integrity rules
- 🐛 **Debug**: Development and troubleshooting tools
- ⚡ **Performance**: Optimization parameters

### **Runtime Configuration**
```cpp
// Update settings during gameplay
UHexGraphSettings* Settings = UHexGraphSettings::GetMutableHexGraphSettings();
Settings->DefaultVertexSpacing = NewSpacing;
Settings->SaveConfig();  // Persist changes

// Refresh objects using settings
SomeHexGraphInstance->RefreshSettingsValues();
```

---

## 📊 **Migration from Hard-Coded Values**

### **Before Configuration System**
```cpp
// Hard-coded values scattered throughout code
maxFillDepth = 5;                    // HexGraph.cpp:43
VertexSpacing = 0.5f;               // HexGraph.cpp:26  
PanSpeed = 15.f;                    // HexGraph.h:262
ZoomPercent = 100.f;                // HexGraph.h:261
MAX_COORDINATE_VALUE = 10000;       // HexGraphValidation.h:143
```

### **After Configuration System**
```cpp
// Centralized configuration with validation
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
maxFillDepth = Settings->MaxFillDepth;
VertexSpacing = Settings->DefaultVertexSpacing;
PanSpeed = Settings->DefaultPanSpeed;
ZoomPercent = Settings->DefaultZoomPercent;
// Validation bounds from settings
```

**Migration Benefits:**
- ✅ **Single Source of Truth**: All configuration in one location
- ✅ **Runtime Modification**: Change values without recompilation
- ✅ **Validation Consistency**: Centralized bounds checking
- ✅ **Team Collaboration**: Shared configuration files

---

## 🚀 **Future Enhancements**

### **Phase 2 Configuration Extensions**
- **Component Settings**: Configuration for new architectural components
- **Performance Profiles**: Preset configurations for different use cases
- **Environment Settings**: Different configs for development/shipping

### **Phase 3 Advanced Configuration**
- **User Preferences**: Player-configurable settings
- **Dynamic Loading**: Hot-reload configuration changes
- **Remote Configuration**: Network-based settings distribution

### **Phase 4 Configuration Analytics**
- **Usage Tracking**: Monitor which settings are changed most often
- **A/B Testing**: Support for configuration experiments
- **Automated Tuning**: Machine learning-based parameter optimization

---

## 🔍 **Best Practices**

### **Settings Design**
- ✅ **Logical Grouping**: Related settings in same category
- ✅ **Clear Names**: Self-documenting property names
- ✅ **Reasonable Defaults**: Safe, functional default values
- ✅ **Validation Rules**: Bounds checking and format validation

### **Runtime Usage**
- ✅ **Cache Settings**: Store frequently-used values locally
- ✅ **Validate Access**: Check for null settings pointer
- ✅ **Fallback Values**: Always provide reasonable defaults
- ✅ **Change Notification**: Update dependent systems when settings change

### **Performance Considerations**
- ✅ **Initialization Time**: Load settings during object construction
- ✅ **Access Frequency**: Cache values for hot paths
- ✅ **Change Detection**: Only update when settings actually change
- ✅ **Batch Updates**: Group related setting changes together

---

**Tags:** #configuration #settings #foundation #udevelopersettings #implemented #claude-generated