# HexGraphSettings

> **Component:** Settings Class  
> **Type:** #configuration #settings #udevelopersettings  
> **Status:** #implemented  
> **Phase:** [[Phase 1]] - Foundation & Stability  
> **Parent:** [[Configuration System]]

## Overview

`UHexGraphSettings` is the core settings class that provides centralized configuration management for the HexGraphMap project. It inherits from `UDeveloperSettings` and integrates with Unreal Engine's Project Settings panel.

**Related Systems:**
- [[Configuration System]] - Complete configuration system architecture
- [[Runtime Configuration]] - Dynamic settings updates and refresh mechanisms
- [[Validation Framework]] - Settings validation and bounds checking

---

## 🎯 **Implementation Details**

### **Class Declaration** - `HexGraphSettings.h:12`
```cpp
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Hex Graph Settings"))
class HEXGRAPHMAP_API UHexGraphSettings : public UDeveloperSettings
```

### **Settings Categories**
- **Graph**: Core gameplay parameters (VertexSpacing, MaxFillDepth, MeshLength)
- **Camera**: View and navigation controls (PanSpeed, ZoomPercent)
- **Memory**: Resource management settings (MaxVertices, CleanupEnabled)
- **Validation**: Data integrity rules (CoordinateBounds, StrictValidation)
- **Debug**: Development tools (MouseFollower, VerboseLogging)
- **Performance**: Optimization parameters (ObjectPooling, PoolSize)

### **Access Interface**
```cpp
// Get settings instance
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();

// Runtime configuration refresh
RefreshSettingsValues();
```

---

## 📖 **Usage Examples**

### **Reading Settings**
```cpp
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
float spacing = Settings ? Settings->DefaultVertexSpacing : 100.0f;
```

### **Runtime Updates**
```cpp
// Update settings and refresh dependent systems
UHexGraphSettings* Settings = UHexGraphSettings::GetMutableHexGraphSettings();
Settings->DefaultVertexSpacing = NewValue;
SomeHexGraphInstance->RefreshSettingsValues();
```

---

## 🔗 **Integration Points**

- **Project Settings**: Edit → Project Settings → Plugins → Hex Graph
- **HexGraph Constructor**: Loads settings during object initialization
- **Validation System**: Provides coordinate bounds and validation rules
- **Memory Management**: Configures cleanup behavior and object limits

---

For complete details, see: [[Configuration System]]

**Tags:** #hexgraphsettings #configuration #udevelopersettings #settings #implemented #claude-generated