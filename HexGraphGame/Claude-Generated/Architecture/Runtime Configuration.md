# Runtime Configuration

> **Component:** Dynamic Settings Management  
> **Type:** #configuration #runtime #dynamic-updates  
> **Status:** #implemented  
> **Phase:** [[Phase 1]] - Foundation & Stability  
> **Parent:** [[Configuration System]]

## Overview

Runtime Configuration provides the ability to update settings during gameplay without requiring recompilation or restart. This system enables dynamic parameter tuning, A/B testing, and live configuration adjustments.

**Related Systems:**
- [[Configuration System]] - Core settings architecture
- [[HexGraphSettings]] - Settings class implementation
- [[Validation Framework]] - Runtime settings validation

---

## 🎯 **Core Features**

### **Dynamic Settings Refresh** - `HexGraph.cpp:472-492`
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
    
    UE_LOG(LogHexGraph, Log, TEXT("RefreshSettingsValues: Updated settings - VertexSpacing: %f, PanSpeed: %f"), 
           VertexSpacing, PanSpeed);
}
```

### **Editor Integration**
Settings can be changed in real-time through:
- **Project Settings Panel**: Edit → Project Settings → Plugins → Hex Graph
- **Console Commands**: Runtime modification via console
- **Blueprint Calls**: `RefreshSettingsValues()` function available

### **Automatic Validation**
All runtime changes are validated through:
- **Settings Validation**: Built-in bounds checking and correction
- **Change Notification**: Logged updates with diagnostic information
- **Fallback Values**: Safe defaults if settings become invalid

---

## 📖 **Usage Patterns**

### **Manual Refresh**
```cpp
// Update settings and refresh object
const UHexGraphSettings* Settings = UHexGraphSettings::GetHexGraphSettings();
SomeHexGraph->RefreshSettingsValues();
```

### **Conditional Updates**
```cpp
// Only update if settings have changed
static float LastVertexSpacing = 0.0f;
if (Settings->DefaultVertexSpacing != LastVertexSpacing)
{
    RefreshSettingsValues();
    LastVertexSpacing = Settings->DefaultVertexSpacing;
}
```

### **Bulk Configuration Changes**
```cpp
// Update multiple settings atomically
UHexGraphSettings* Settings = UHexGraphSettings::GetMutableHexGraphSettings();
Settings->DefaultVertexSpacing = NewSpacing;
Settings->MaxFillDepth = NewDepth;
Settings->SaveConfig();  // Persist changes
RefreshSettingsValues(); // Apply to runtime objects
```

---

## 🚀 **Future Enhancements**

### **Phase 2 Extensions**
- **Change Notifications**: Observer pattern for settings changes
- **Hot Reload**: Automatic refresh when settings files change
- **Configuration Profiles**: Named presets for different use cases

### **Phase 3 Advanced Features**
- **Network Configuration**: Remote settings updates for multiplayer
- **A/B Testing**: Runtime configuration experiments
- **User Preferences**: Player-specific settings overrides

---

For complete implementation details, see: [[Configuration System]]

**Tags:** #runtime-configuration #dynamic-settings #configuration #real-time #implemented #claude-generated