// ==WindhawkMod==
// @id              taskbar-multi-tray
// @name            Taskbar multi-tray
// @description     Windows 11 taskbar tray/control-center visibility controls for all or selected monitors
// @version         1.0.0
// @author          EDM115
// @github          https://github.com/EDM115
// @twitter         https://twitter.com/_edm115
// @homepage        https://edm115.dev/
// @include         explorer.exe
// @include         ShellHost.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lcomctl32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `taskbar-multi-tray`
## Windows 11 Windhawk mod for controlling taskbar tray and control-center visibility across multi-monitor setups
The mod hooks Explorer's taskbar XAML host, selected taskbar.dll taskbar/tray entry points, ShellHost flyout monitor helpers, and a small set of monitor/window-placement APIs. It can apply tray/control-center visibility rules to every taskbar or only to selected monitor numbers.

## What it controls
**Tray** means the Windows 11 taskbar notification area : promoted tray icons, the hidden-icons entry point, and the XAML containers Windows creates for them.  
**Control center** means the Windows 11 control-center button (connection status, volume, battery level, ...) *without* the notification/date-time button which Windows can already hide and display on all monitors (we don't handle that one). Together they form what Windows calls the "Action center" surface. We also don't target (yet) the aero peek button.  
The `components` setting chooses which surfaces the mod keeps visible :
- `all` : tray icons, hidden icons, and control center
- `tray` : tray icons and hidden icons, the notification/date-time button stays visible because Windows treats it as part of the date/time surface, while the control-center button is hidden
- `controlCenter` : control center and notification/date-time only, promoted tray icons and the hidden-icons entry point are hidden

## Settings
- `monitorMode` : `all` or `selected`
- `selectedMonitors` : comma-separated one-based monitor numbers, used only with `monitorMode=selected`
- `components` : `all`, `tray`, or `controlCenter`
- `enableVerboseLogging` : enables all runtime logging, when disabled, the mod does not write Windhawk log entries
- `enableTreeDump` : adds bounded XAML child-tree dumps only when verbose logging is also enabled

Monitor numbering follows `EnumDisplayMonitors` order for the current session. The numbers can change after hot-plug, docking, display sleep, or reboot.

## Demo screenshots
![enable-everywhere-tray-apps](https://i.imgur.com/C0VTSGB.jpeg)
![enable-everywhere-control-center](https://i.imgur.com/UVH38vQ.jpeg)
![tray-select](https://i.imgur.com/3yDk1vi.jpeg)
![control-select](https://i.imgur.com/s9l2chj.jpeg)

## Compatibility and safety
This mod targets **only** Windows 11, and **only** the newer versions.  
It cannot be applied to the Windows 10 taskbar as-is. Windows 10 exposes the notification area through the older Explorer/Win32 taskbar window hierarchy, with classes such as `Shell_TrayWnd`, `TrayNotifyWnd`, pager/toolbar controls, and the classic notification-area icon model. This mod instead targets the Windows 11 taskbar implementation : XAML/WinUI-hosted taskbar surfaces and Windows 11-specific elements such as `SystemTrayFrame`, `NotifyIconStack`, `NotificationAreaIcons`, `ControlCenterButton`, and ShellHost-hosted flyouts. Those objects, bindings, and flyout paths simply do not exist in the same form on Windows 10.  
The old "Windows 10 taskbar on Windows 11" approach worked differently : tools such as [ExplorerPatcher](https://github.com/valinet/ExplorerPatcher/wiki/ExplorerPatcher%27s-taskbar-implementation) could rely on Microsoft still shipping enough of the legacy Windows 10 taskbar code inside earlier Windows 11 builds, then switch Explorer back to that legacy path. For reference, Windows 11 version 22H2 is OS build `22621`, which users can check with `winver`. Starting with the newer Windows 11 24H2 line (OS build `26100`), that stock legacy taskbar path is no longer available in the same way, so ExplorerPatcher moved to a custom reimplemented Windows 10-style taskbar based mostly on the code present in Windows 11 22H2. This mod does not reimplement a taskbar, it modifies the native Windows 11 XAML taskbar that is already running.  
Compatibility with other mods that touch the taskbar isn't guaranteed. Feel free to report an issue in case of incompatibility, crash or any other bug. In that case, enable verbose logging and provide the relevant log entries.  
The mod is silent unless `enableVerboseLogging` is enabled. Verbose logging records settings, taskbar selection, XAML element state, binding reuse, proxy/native flyout contexts, monitor redirection, placement translation, hook availability, startup retries, and cleanup. `enableTreeDump` only adds bounded XAML child-tree dumps when verbose logging is also enabled.

### Known issues
*Those are problems that are identified and will be fixed in future updates*
- [ ] Flyouts (hidden tray apps or control center) can open on the wrong monitor, this is caused by Windows ignoring the click on the proxied controls as a "monitor interaction"
- [ ] On rare occasions, one of the taskbars can crash, in which case it restarts automatically
- [ ] The mod has a risk of not being applied at startup, in which case disable and re-enable it

#### Fixed bugs (in antichronological order)
- [x] The mod hides the Start Menu Search panel content as it targets `ShellHost.exe` as well. It stays fully functional tho, with mouse and keyboard navigation staying intact.
- [x] The taskbar crashes when right-clicking the control center as it doesn't handle the spawned menu
- [x] The taskbar can crash when switching quickly between monitors with mixed DPI
- [x] The mod can have a delay between a click on a monitor and it being registered as an interaction, which can cause the flyout to open on the wrong monitor if the user clicks on a different monitor during that time
- [x] Disabling the mod stalls in `explorer.exe` with it taking a minute before timing out, and then crashing the taskbars despite not being interactive after another minute
- [x] The taskbar(s) sometimes can't appear on monitors where an application is in full-screen mode, including windowed borderless. It can however appear on the full-screened one, given that focus is on the other monitors.
- [x] Action center (and app tray) can be offset when the notifications bell icon is enabled
- [x] The mod can use more resources than necessary (notably when interacting with the proxied surfaces quickly between multiple monitors)
- [x] Opening the control center refreshes all taskbars
- [x] Order of monitors is non-deterministic and can change the primary surface location
- [x] Switching from all monitors to a selection of monitors leaves non-selected monitors in a broken state where the proxied surfaces still appear but are not interactive
- [x] The hidden tray icons flyout can appear offset if multiple monitors have different resolutions/DPI/zoom levels
- [x] Disabling the mod doesn't restore the original taskbars states and leave them in a non-functional states, as there are no listeners for the proxied controls
- [x] The clicks on proxied controls could fail if clicked on the edge of said controls
- [x] The hidden tray arrow doesn't duplicate
- [x] The mod overrides the notification/datetime flyout location, Windows should keep control over it as it already does work well
- [x] The mod also hides the notification/datetime button when in tray-only mode, as Windows treats it as part of the action center surface, tho it's out of scope of the mod as it's already handled by Windows
- [x] The control center button is empty but interactive and clicking on it switches the primary taskbar to the target monitor
- [x] The mod only moves the actual areas singleton to another monitor, as if the primary display is changed

---

## Changelog
### 1.0.0
**Summary :** Initial public release after testing and polishing  
**Details :** Reformat the code, make sure logs appear only when verbose logging is enabled, add a proper changelog, rework the documentation, add a bugs list and publish the mod

### 0.30
**Summary :** Fixed regressions from rapid-switch hardening and made unload safer  
**Details :** Kept the generation/timer race fix but narrowed active monitor forcing so Explorer no longer redirects unrelated top-level windows while a tray/action flyout is open. Active flyout redirection is suppressed while the mod applies or restores taskbar XAML, shared ShellHost state is published in two phases to avoid mixed monitor/DPI snapshots, deferred startup retries use taskbar-window timers instead of detached worker threads, unload cancels deferred/native timers and removes taskbar subclasses before restore, and `GetMessageW`/`PeekMessageW` hooks were replaced with a safer `DispatchMessageW` context-menu guard.

### 0.29
**Summary :** Stabilized rapid multi-monitor flyout switching after validation  
**Details :** Added a generation id to native-click flyout contexts so stale timers from a previous monitor cannot clear a newer click. Stale active context is cleared before a left click on a different taskbar monitor, closing and pure z-order placement calls are kept out of the flyout-placement translator, and the post-placement monitor context is shortened to reduce stale ShellHost flyout interference.

### 0.28
**Summary :** Fixed the main reported flyout-placement, mixed-DPI, Search, right-click, and startup-apply issues  
**Details :** Active native flyout monitor forcing was widened during the short clicked-taskbar context so ShellHost could not prefer the last active screen. Hidden-icons hit testing and anchoring were scaled to clicked-taskbar DPI, ShellHost primary-display spoofing was gated to active tray/action flyouts, right-click/context-menu messages clear the temporary context from Explorer and ShellHost message loops, and Explorer schedules deferred startup apply retries when the taskbar/XAML tree is not ready yet.

### 0.27
**Summary :** Added DPI-scaled placement and narrower active forcing  
**Details :** Translated flyout rectangles are scaled to the clicked taskbar DPI, scaled sizes are passed back through the window-position hooks, active flyout `MonitorFromPoint` and `MonitorFromRect` forcing is narrowed to ambiguous or target-side queries, and temporary flyout context is cleared before taskbar right-click context menus.

### 0.26
**Summary :** Corrected regressions from the hidden-icons anchoring change  
**Details :** The overflow anchor is used only for hidden-icons clicks, not control-center clicks. The flyout is positioned above the clicked taskbar rectangle with a small gap, and taskbar monitor queries are not forced for a different real monitor while an older flyout context is active.

### 0.25
**Summary :** Fixed mixed-DPI hidden-icons overflow anchoring  
**Details :** Hidden-icons overflow placement began anchoring to the clicked chevron's screen position and the target monitor work-area bottom instead of preserving a primary-monitor source popup offset. The native flyout context was shortened so stale monitor state was less likely to affect later clicks.

### 0.24
**Summary :** Scoped native-click context to avoid inherited stale monitors  
**Details :** The clicked-monitor context is armed from the real, unredirected taskbar monitor. `MonitorFromWindow` forcing was limited to taskbar windows and known flyout popup windows, reducing the chance that a previous flyout context would affect the next unrelated taskbar click.

### 0.23
**Summary :** Added explicit popup placement translation  
**Details :** Some flyouts ignored monitor-query results and applied explicit coordinates. The mod began translating `TopLevelWindowForOverflowXamlIsland` and `ControlCenterWindow` placement calls to the clicked monitor via `SetWindowPos`, `MoveWindow`, `SetWindowPlacement`, and `DeferWindowPos`.

### 0.22
**Summary :** Broadened the temporary native-click monitor context  
**Details :** During copied control-center or hidden-icons opens, `MonitorFromPoint`, `MonitorFromRect`, and `MonitorFromWindow` all resolved to the clicked taskbar's monitor. Hit-test tolerance was added around copied native controls so edge clicks still armed the correct monitor context.

### 0.21
**Summary :** Let hidden-icons clicks stay native and copied the inner list binding  
**Details :** Stopped swallowing hidden-icons clicks, hooked `TaskbarHost::ActivateOverflowFlyout` as a passive diagnostic, copied the inner `NotifyIconStackListView` binding that appears to own hidden-icons content, and added native taskbar restoration on mod disable.

### 0.20
**Summary :** Added a direct hidden-icons activation experiment  
**Details :** Forced and logged a bounded `NotifyIconStack` descendant tree, then called `TaskbarHost::ActivateOverflowFlyout` on hidden-icons clicks. This isolated whether the missing chevron was a visual-template issue or whether the taskbar host could open the overflow flyout without moving the singleton tray.

### 0.19
**Summary :** Added native-click flyout monitor context for copied controls  
**Details :** Native control-center clicks now arm a clicked-monitor context before falling through, allowing ShellHost to open on the clicked monitor without refreshing all taskbars. The hidden-icons move-and-click proxy was stopped, the first `NotifyIconStack` child was forced visible, and hidden-icons clicks were allowed to fall through with the same monitor context.

### 0.18
**Summary :** Stopped proxying secondary control-center clicks  
**Details :** After copied control-center XAML content became usable, secondary control-center clicks were left to the native XAML click path instead of the move-and-click proxy path. This reduced unnecessary real-tray movement for control-center opens.

### 0.17
**Summary :** Made selected-monitor ordering explicit and reset skipped taskbars  
**Details :** Preserved the original order from `selectedMonitors`, the first selected monitor became the singleton real-tray owner. The apply path processes the real tray source first, resets non-selected taskbars to their default secondary state, and adds first-child binding diagnostics for `NotifyIconStack`.

### 0.16
**Summary :** Reused more primary tray/action-center XAML bindings  
**Details :** Generalized binding capture and reuse to `NotifyIconStack` and `ControlCenterButton`, allowing the hidden-icons entry point and the visual control-center content to be tested with the same binding strategy used for promoted tray icons.

### 0.15
**Summary :** Added XAML-level promoted-icon binding reuse  
**Details :** Cached the populated primary `NotificationAreaIcons` binding and applied it to width-collapsed secondary controls when useful. This tested a less invasive duplication path at the XAML binding layer instead of copying native icon-manager ownership.

### 0.14
**Summary :** Made optional diagnostic symbols best-effort  
**Details :** Fixed a regression where missing optional notification-area diagnostic symbols could prevent the core Explorer-side hooks from loading. Required taskbar hooks are installed first, lower notification-area probes are hooked only when their symbols are available.

### 0.13
**Summary :** Added passive notification-area model diagnostics  
**Details :** Hooked `TaskbarModel::SetNotificationAreaIconManager2`, promoted/overflow icon getters, visible-icon collection mutations, and `NotificationAreaIconManager2::ShellNotifyIcon` as diagnostics. These hooks recorded primary/secondary manager activity without replacing or sharing the native manager, avoiding unsafe ownership changes inside Explorer.

### 0.12
**Summary :** Tested primary-like secondary `TaskbarHost` construction  
**Details :** Moved the secondary initialization experiment deeper by forcing the `TaskbarHost` constructor's primary-like flag while selected secondary trays were being created. Added logging around `SystemTrayHost::TryLoadNotificationAreaFrameFromXamlExtension` to determine whether the missing multi-monitor tray was blocked at host construction or lower in the singleton notification-area model.

### 0.11
**Summary :** Tested secondary-tray initialization monitor spoofing  
**Details :** Added thread-local secondary initialization monitor state around `CSecondaryTray::InitModelAndHost`. While a selected secondary tray initialized, monitor queries and primary-display selection could be spoofed to that secondary monitor to test whether Windows would initialize a fuller tray surface when it believed the secondary host was primary-like.

### 0.10
**Summary :** Added ShellHost injection and shared proxy state  
**Details :** Added `ShellHost.exe` as an injection target, process detection, and a shared data section carrying runtime tray/flyout monitor state between Explorer and ShellHost. Monitor query and primary-display spoofing began working in ShellHost too, which covered newer Windows 11 paths where `Win+A` and Control Center are hosted outside Explorer. The proxy path also learned to restore the tray target after temporary opens.

### 0.9
**Summary :** Split verbose logging from XAML tree dumps and widened flyout redirection  
**Details :** Added `enableTreeDump`, gated tree dumps behind that setting, hooked `ImmersiveMonitorHelper::ConnectToMonitor` directly, logged proxy hit tests, extended the proxy context lifetime, and deduplicated repeated proxy posts. This covered Windows builds that place notification and control-center flyouts through different immersive monitor paths.

### 0.8
**Summary :** Added secondary action-center proxy clicks and immersive flyout monitor hook for proxy opens  
**Details :** Added taskbar subclassing, secondary-taskbar hit testing from the right edge, proxy messages, `SendInput` hotkey/click helpers, and temporary runtime primary-tray monitor state. Secondary notification/date and control-center clicks could ask Explorer to move the real tray target to the clicked monitor, redirect default monitor queries, and open the matching Windows flyout. The notification/date surface was kept visible in tray mode because Windows already exposes it on secondary taskbars. Added `twinui.pcshell.dll` symbol hooks for `ImmersiveMonitorHelper::AdjustMonitorConnectedIfNeeded` and access to `ConnectToMonitor`. During a proxy-open window, the hook connected the flyout helper to the clicked monitor center point, improving placement for flyouts that did not follow the simpler monitor-query redirects.

### 0.7
**Summary :** Made component filtering hide unwanted surfaces explicitly  
**Details :** Added width reset helpers and `SetElementVisibility` so disabled component groups are collapsed and non-hit-testable instead of merely not forced visible. `tray` and `actionCenter` modes became true filters : tray mode hides the control-center button while action-center mode hides the tray icon and hidden-icons surfaces.

### 0.6
**Summary :** Added real primary tray monitor movement for selected-monitor mode  
**Details :** Hooked `TrayUI::_SetStuckMonitor`, added monitor lookup by one-based index, and notified Explorer of taskbar display changes so Windows would re-run stuck-monitor logic. With `monitorMode=selected`, the mod could ask Windows to move the singleton real tray/action-center surface to the selected monitor instead of only applying XAML visibility rules.

### 0.5
**Summary :** Improved layout forcing for collapsed tray/action-center elements  
**Details :** Added best-effort `UpdateLayout`, minimum-width forcing, exact-width fallback for collapsed controls, and frame min-width calculation. The code switched from generic stack names to more precise Windows 11 tray elements such as `NotifyIconStack`, `NotificationAreaIcons`, `ControlCenterButton`, and `NotificationCenterButton`, and reduced tree-dump depth to keep diagnostics bounded.

### 0.4
**Summary :** Added detailed diagnostics for taskbar and XAML state  
**Details :** Introduced readable setting/component/visibility formatting, taskbar window logging with monitor number and display device, XAML element state logging, and bounded visual-tree dumping. The apply path began logging missing elements and inspecting `XamlRoot.Content`, `SystemTrayFrame`, and tray/action-center children so secondary-taskbar failures could be diagnosed from logs instead of guesses.

### 0.3
**Summary :** Initial experimental Windows 11 taskbar XAML implementation  
**Details :** Added Explorer-only injection, monitor selection settings, component filtering settings, and the first XAML traversal path for taskbar tray/action-center elements. The implementation used Windhawk taskbar/XAML hook patterns to find `SystemTrayFrame` and related children, then forced selected controls visible for all taskbars or selected monitor numbers.

### 0.2
**Summary :** Fixed the worker-thread model, popup handling, and metadata
**Details :** Reworked settings and background refresh around mutex-protected snapshots, a condition-variable worker loop, and explicit wakeups on settings changes, avoiding direct unsynchronized access to runtime settings. Monitor matching was made more efficient with a per-apply monitor-index map, selected-monitor filtering was moved to immutable apply context data, and popup/flyout classes were removed from the persistent tray child list so the periodic refresh only touched stable tray components. The update also removed unnecessary compiler options/includes, added x64 architecture documentation, clarified live Windhawk reload behavior, switched metadata to the final author/homepage fields, and kept verbose logging documented as a debugging-only setting.

### 0.1
**Summary :** Added the first experimental Win32 taskbar-tray scaffold
**Details :** Introduced the initial Windhawk mod, targeting `explorer.exe` with a defensive window-enumeration strategy instead of private taskbar symbol hooks. The first implementation added `mode`, `selectedMonitors`, `applyIntervalMs`, and `enableVerboseLogging` settings, enumerated monitors and taskbar windows, parsed selected monitor lists, and periodically showed persistent tray-related child windows such as `TrayNotifyWnd`, clock classes, overflow windows, and input indicators. It also documented the initial safety assumptions, monitor numbering, and best-effort development workflow.

---

## How it works
Windows 11 keeps the real notification-area model and the full primary tray surface as a singleton. This mod does not create a second native tray icon manager and does not copy Explorer's internal `std::shared_ptr` ownership. Instead, it works with the taskbar and XAML surfaces Windows already creates.  
On compatible taskbars, the mod adjusts XAML visibility, width, hit testing, and selected bindings for elements such as `SystemTrayFrame`, `SystemTrayFrameGrid`, `NotifyIconStack`, `NotificationAreaIcons`, `ControlCenterButton`, and `NotificationCenterButton`. If a selected secondary taskbar has no usable tray frame or model content, the mod leaves that taskbar alone.  
For selected-monitor mode, the first valid monitor number in `selectedMonitors` is treated as the preferred owner of Windows' singleton real tray/control-center surface. For example, `selectedMonitors=3,1` prefers monitor 3, while `selectedMonitors=1,3` prefers monitor 1. Additional selected monitors still receive XAML filtering and copied bindings where Windows exposes compatible elements, but the real native tray surface is not duplicated.  
Secondary control-center and hidden-icons clicks use the native XAML click path when possible. Before letting the native click continue, the mod arms a temporary clicked-monitor context so Explorer and ShellHost can resolve the flyout to the clicked taskbar's monitor. During that short context it redirects monitor queries only for ambiguous geometry, the clicked taskbar, and known flyout popup windows such as `ControlCenterWindow` and `TopLevelWindowForOverflowXamlIsland`.  
The hidden-icons path also copies the inner `SystemTray.StackListView` binding from the real tray owner when available. That allows a secondary chevron to use the primary tray owner's item source without replacing the native notification-area icon manager.  
The notification/date button can use a proxy path. The proxy temporarily points the real tray target at the clicked taskbar monitor, opens the matching Windows flyout, then restores the normal target when appropriate.  
ShellHost monitor/display spoofing is limited to active tray/control-center flyout opens. Unrelated ShellHost surfaces, including Windows Search, keep their native placement.  
On settings changes or disable, the mod cancels pending taskbar timers, removes taskbar subclasses, clears cached XAML bindings and shared flyout state, and asks Explorer to restore the native primary taskbar state.

---

## Development setup
1. Install Windhawk from https://windhawk.net/
2. Open Windhawk, create a new mod, and paste `taskbar-multi-tray.wh.cpp`
3. Compile the mod
4. Configure settings :
   - `monitorMode=all`, or `monitorMode=selected` with `selectedMonitors=3,1`
   - `components=all`, `components=tray`, or `components=controlCenter`
   - `enableVerboseLogging=1` (and `enableTreeDump=1` if needed for XAML layout debugging) 
5. Usually no Explorer restart is needed, Windhawk can inject and apply live. If the taskbar does not refresh immediately, reload the mod or restart `explorer.exe`.

## Debugging
Keep `enableVerboseLogging` disabled unless actively debugging. With verbose logging enabled, the mod logs settings, taskbar selection, XAML element state, binding reuse, proxy/native flyout contexts, monitor redirection, placement translation, hook availability, startup retries, and cleanup. `enableTreeDump` adds the noisy bounded XAML child tree and should stay disabled unless the element layout itself is being investigated.  
Useful log filters when diagnosing missing tray/control-center content :
- `ApplyStyle hwnd=`
- `SystemTrayFrame`
- `SystemTrayFrameGrid`
- `NotifyIconStack`
- `NotificationAreaIcons`
- `NotificationAreaIcons binding`
- `NotifyIconStack binding`
- `NotifyIconStackChild binding`
- `NotifyIconStackListView binding`
- `hidden-stack child`
- `TaskbarHost::ActivateOverflowFlyout hook`
- `ControlCenterButton binding`
- `applied primary NotificationAreaIcons binding`
- `applied primary NotifyIconStack binding`
- `applied primary NotifyIconStackChild binding`
- `applied primary NotifyIconStackListView binding`
- `applied primary ControlCenterButton binding`
- `cached promoted icon width`
- `resetting skipped monitor`
- `ControlCenterButton`
- `NotificationCenterButton`
- `width-collapsed`
- `moving real primary tray`
- `restoring real primary tray`
- `native restore target monitor`
- `restoring native taskbar XAML state`
- `not forcing MonitorFromWindow`
- `removed subclass/timers`
- `DispatchMessageW`
- `scheduled deferred apply retry`
- `deferred apply retry from taskbar timer`
- `secondary click`
- `control center native`
- `hidden tray native`
- `native control center flyout monitor context`
- `native hidden tray flyout monitor context`
- `anchor=1`
- `kind=4`
- `taskbar=`
- `dpi=`
- `anchored hidden tray flyout`
- `shortening active flyout monitor context`
- `ignoring stale native flyout context timer`
- `clearing stale flyout monitor context before taskbar click`
- `deferred apply retry`
- `before right-click/context menu message`
- `not forcing MonitorFromWindow`
- `not forcing MonitorFromPoint`
- `not forcing MonitorFromRect`
- `clearing flyout monitor context before right-click/context menu`
- `clearing native flyout monitor context`
- `proxy opening`
- `clicking moved control center`
- `restoring proxy tray target`
- `redirecting MonitorFromPoint`
- `redirecting MonitorFromRect`
- `forcing MonitorFromPoint`
- `forcing MonitorFromRect`
- `forcing MonitorFromWindow`
- `moving TopLevelWindowForOverflowXamlIsland`
- `moving ControlCenterWindow`
- `marking`
- `forcing immersive flyout`
- `redirecting ConnectToMonitor`
- `hooked twinui.pcshell.dll symbols`
- `spoofing secondary tray init`
- `TaskbarHost ctor`
- `SystemTrayHost::TryLoadNotificationArea`
- `primary-like TaskbarHost flag`
- `hooked optional taskbar symbol`
- `optional taskbar symbol unavailable`
- `SetNotificationAreaIconManager2`
- `get_NotificationAreaPromotedIcons`
- `get_NotificationAreaOverflowIcons`
- `NotificationAreaIconManager2::AddIconToVisibleCollection`
- `NotificationAreaIconManager2::ShellNotifyIcon`
- `secondary tray window unavailable`
- `can't duplicate the real primary tray model`

The Windows 11 tray containers can exist on secondary taskbars while their internal icon model is still empty. A visibility-only XAML tweak cannot synthesize missing tray icon content, the mod can only expose or reuse surfaces Windows has made available. 
Windhawk debugging references :
- Creating mods : https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod
- Debugging mods : https://github.com/ramensoftware/windhawk/wiki/Debugging-the-mods
- Development tips : https://github.com/ramensoftware/windhawk/wiki/Development-tips
- Injection targets and critical processes : https://github.com/ramensoftware/windhawk/wiki/Injection-targets-and-critical-system-processes
- Mod lifetime : https://github.com/ramensoftware/windhawk/wiki/Mod-lifetime
- Reference mods : https://github.com/ramensoftware/windhawk-mods/tree/main/mods

## Attribution
- Author : [@EDM115](https://github.com/EDM115) with great help from [Codex](https://chatgpt.com/codex)
- Idea : [@rutaaa](https://github.com/rutaaa) (no he didn't bully me into doing this 😭)
- License : MIT
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- monitorMode: all
  $name: Monitors
  $options:
  - all: All monitors
  - selected: Selected monitors
- selectedMonitors: "1"
  $name: Selected monitors
  $description: >-
    Comma-separated monitor numbers (ex : 1,3), used only when Monitors is set to Selected monitors
- components: all
  $name: Components
  $options:
  - all: Tray and control center
  - tray: Tray icons and hidden icons
  - controlCenter: Control center only
- enableVerboseLogging: false
  $name: Enable verbose logging
- enableTreeDump: false
  $name: Dump XAML tree
  $description: >-
    Very noisy, used only with verbose logging when investigating missing XAML elements
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#undef GetCurrentTime

#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>

#include <cwctype>
#include <functional>
#include <limits>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace winrt::Windows::UI::Xaml;

enum class MonitorMode {
    All,
    Selected,
};

enum class Components {
    All,
    Tray,
    ControlCenter,
};

struct Settings {
    MonitorMode monitorMode = MonitorMode::All;
    std::set<int> selectedMonitors;
    std::vector<int> selectedMonitorOrder;
    Components components = Components::All;
    bool enableVerboseLogging = false;
    bool enableTreeDump = false;
};

Settings g_settings;

#define Wh_Log_safe(message, ...) \
    do { \
        if (g_settings.enableVerboseLogging) { \
            Wh_Log(message __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while (false)

enum class TargetProcess {
    Explorer,
    ShellHost,
    Other,
};

TargetProcess g_targetProcess = TargetProcess::Other;

constexpr WPARAM kProxyControlCenter = 1;
constexpr WPARAM kProxyNotificationCenter = 2;
constexpr WPARAM kNativeControlCenter = 3;
constexpr WPARAM kNativeHiddenTray = 4;

constexpr DWORD kNativeFlyoutMonitorContextMs = 5000;
constexpr DWORD kNativeFlyoutMonitorContextAfterPlacementMs = 650;
constexpr DWORD kDeferredApplyRetryDelaysMs[] = {750, 1500, 3000, 6000};

asm(".section .shared,\"dws\"\n");
#define SHARED_SECTION __attribute__((section(".shared")))

struct SharedProxyState {
    HMONITOR runtimePrimaryTrayMonitor;
    int runtimePrimaryTrayMonitorIndex;
    HMONITOR proxyFlyoutMonitor;
    int proxyFlyoutMonitorIndex;
    DWORD proxyFlyoutUntilTick;
    LONG proxyFlyoutGeneration;
    int proxyFlyoutKind;
    LONG_PTR proxyFlyoutTaskbarWnd;
    int proxyFlyoutAnchorValid;
    LONG proxyFlyoutAnchorX;
    LONG proxyFlyoutAnchorY;
    int proxyFlyoutTaskbarRectValid;
    LONG proxyFlyoutTaskbarLeft;
    LONG proxyFlyoutTaskbarTop;
    LONG proxyFlyoutTaskbarRight;
    LONG proxyFlyoutTaskbarBottom;
    UINT proxyFlyoutTargetDpi;
};

SharedProxyState g_sharedProxyState SHARED_SECTION = {};

void* CTaskBand_ITaskListWndSite_vftable;
void* CSecondaryTaskBand_ITaskListWndSite_vftable;

using CTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void* pThis, void** result);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;

using CSecondaryTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void* pThis, void** result);
CSecondaryTaskBand_GetTaskbarHost_t CSecondaryTaskBand_GetTaskbarHost_Original;

void* TaskbarHost_FrameHeight_Original;

using TaskbarHost_ActivateOverflowFlyout_t = void(WINAPI*)(void* pThis);
TaskbarHost_ActivateOverflowFlyout_t TaskbarHost_ActivateOverflowFlyout_Original;

using std__Ref_count_base__Decref_t = void(WINAPI*)(void* pThis);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

using TrayUI__SetStuckMonitor_t = HRESULT(WINAPI*)(void* pThis, HMONITOR monitor);
TrayUI__SetStuckMonitor_t TrayUI__SetStuckMonitor_Original;

using TaskbarModel_SetNotificationAreaIconManager2_t = void(WINAPI*)(void* pThis, void* notificationAreaIconManagerSharedPtr);
TaskbarModel_SetNotificationAreaIconManager2_t TaskbarModel_SetNotificationAreaIconManager2_Original;

using TaskbarModel_GetNotificationAreaIcons_t = HRESULT(WINAPI*)(void* pThis, void** result);
TaskbarModel_GetNotificationAreaIcons_t TaskbarModel_GetNotificationAreaPromotedIcons_Original;
TaskbarModel_GetNotificationAreaIcons_t TaskbarModel_GetNotificationAreaOverflowIcons_Original;

using NotificationAreaIconManager2_IconMethod_t = void(WINAPI*)(void* pThis, void* icon);
NotificationAreaIconManager2_IconMethod_t NotificationAreaIconManager2_AddIconToVisibleCollection_Original;
NotificationAreaIconManager2_IconMethod_t NotificationAreaIconManager2_RemoveIconFromVisibleCollection_Original;

using NotificationAreaIconManager2_ShellNotifyIcon_t = bool(WINAPI*)(void* pThis, void* trayNotifyData);
NotificationAreaIconManager2_ShellNotifyIcon_t NotificationAreaIconManager2_ShellNotifyIcon_Original;

using MonitorFromPoint_t = decltype(&MonitorFromPoint);
MonitorFromPoint_t MonitorFromPoint_Original;

using MonitorFromRect_t = decltype(&MonitorFromRect);
MonitorFromRect_t MonitorFromRect_Original;

using MonitorFromWindow_t = decltype(&MonitorFromWindow);
MonitorFromWindow_t MonitorFromWindow_Original;

using SetWindowPos_t = decltype(&SetWindowPos);
SetWindowPos_t SetWindowPos_Original;

using MoveWindow_t = decltype(&MoveWindow);
MoveWindow_t MoveWindow_Original;

using SetWindowPlacement_t = decltype(&SetWindowPlacement);
SetWindowPlacement_t SetWindowPlacement_Original;

using DeferWindowPos_t = decltype(&DeferWindowPos);
DeferWindowPos_t DeferWindowPos_Original;

using EnumDisplayDevicesW_t = decltype(&EnumDisplayDevicesW);
EnumDisplayDevicesW_t EnumDisplayDevicesW_Original;

using DispatchMessageW_t = decltype(&DispatchMessageW);
DispatchMessageW_t DispatchMessageW_Original;

HMONITOR g_runtimePrimaryTrayMonitor = nullptr;
int g_runtimePrimaryTrayMonitorIndex = 0;
bool g_restoringNativeTaskbars = false;
HMONITOR g_nativePrimaryRestoreMonitor = nullptr;
HMONITOR g_proxyFlyoutMonitor = nullptr;
int g_proxyFlyoutMonitorIndex = 0;
DWORD g_proxyFlyoutUntilTick = 0;
LONG g_proxyFlyoutGeneration = 0;
int g_proxyFlyoutKind = 0;
HWND g_proxyFlyoutTaskbarWnd = nullptr;
bool g_proxyFlyoutAnchorValid = false;
POINT g_proxyFlyoutAnchorPoint = {};
bool g_proxyFlyoutTaskbarRectValid = false;
RECT g_proxyFlyoutTaskbarRect = {};
UINT g_proxyFlyoutTargetDpi = 0;
HWND g_lastProxyPostWnd = nullptr;
WPARAM g_lastProxyPostFlyout = 0;
DWORD g_lastProxyPostTick = 0;
volatile LONG g_deferredApplyGeneration = 0;
volatile LONG g_modUnloading = 0;
volatile LONG g_activeFlyoutRedirectionSuppressionDepth = 0;
HWND g_deferredApplyTimerWnd = nullptr;
LONG g_deferredApplyTimerGeneration = 0;
size_t g_deferredApplyRetryIndex = 0;
thread_local HMONITOR g_secondaryInitMonitor = nullptr;
thread_local int g_secondaryInitMonitorIndex = 0;
thread_local bool g_inSecondaryTrayInit = false;
thread_local bool g_secondaryTaskbarHostPrimaryLike = false;
void* g_lastPrimaryNotificationAreaIconManager = nullptr;
void* g_lastSecondaryNotificationAreaIconManager = nullptr;

struct CachedXamlBinding {
    winrt::Windows::Foundation::IInspectable dataContext = nullptr;
    winrt::Windows::Foundation::IInspectable itemsSource = nullptr;
    winrt::Windows::Foundation::IInspectable content = nullptr;
};

CachedXamlBinding g_primaryNotificationAreaIconsBinding;
CachedXamlBinding g_primaryNotifyIconStackBinding;
CachedXamlBinding g_primaryNotifyIconStackChildBinding;
CachedXamlBinding g_primaryNotifyIconStackListViewBinding;
CachedXamlBinding g_primaryControlCenterButtonBinding;

struct TaskbarTrayMetrics {
    HWND hWnd = nullptr;
    double notificationAreaIconsWidth = 0.0;
};

std::vector<TaskbarTrayMetrics> g_taskbarTrayMetrics;

struct NativeFlyoutTimerState {
    HWND hWnd = nullptr;
    LONG generation = 0;
};

std::vector<NativeFlyoutTimerState> g_nativeFlyoutTimerStates;

bool IsModUnloading() {
    return g_modUnloading != 0;
}

bool IsActiveFlyoutRedirectionSuppressed() {
    return IsModUnloading() || g_activeFlyoutRedirectionSuppressionDepth > 0;
}

struct ActiveFlyoutRedirectionSuppressor {
    ActiveFlyoutRedirectionSuppressor() {
        InterlockedIncrement(&g_activeFlyoutRedirectionSuppressionDepth);
    }

    ~ActiveFlyoutRedirectionSuppressor() {
        InterlockedDecrement(&g_activeFlyoutRedirectionSuppressionDepth);
    }
};

double GetCachedNotificationAreaIconsWidth(HWND hWnd);
void SetCachedNotificationAreaIconsWidth(HWND hWnd, double width);
void RemoveCachedTaskbarTrayMetrics(HWND hWnd);
bool IsSecondaryTaskbarWindow(HWND hWnd);
std::wstring GetWindowClassName(HWND hWnd);

using ImmersiveMonitorHelper_ConnectToMonitor_t = bool(WINAPI*)(void* pThis, HWND hWnd, POINT point);
ImmersiveMonitorHelper_ConnectToMonitor_t ImmersiveMonitorHelper_ConnectToMonitor_Original;

using ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_t = HRESULT(WINAPI*)(void* pThis);
ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_t ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Original;

PCWSTR MonitorModeToString(MonitorMode mode) {
    return mode == MonitorMode::Selected ? L"selected" : L"all";
}

PCWSTR ComponentsToString(Components components) {
    switch (components) {
        case Components::Tray:
            return L"tray";
        case Components::ControlCenter:
            return L"controlCenter";
        case Components::All:
        default:
            return L"all";
    }
}

PCWSTR VisibilityToString(Visibility visibility) {
    switch (visibility) {
        case Visibility::Visible:
            return L"Visible";
        case Visibility::Collapsed:
            return L"Collapsed";
        default:
            return L"Unknown";
    }
}

TargetProcess GetTargetProcess() {
    wchar_t modulePath[MAX_PATH] = {};

    if (!GetModuleFileNameW(nullptr, modulePath, ARRAYSIZE(modulePath))) {
        return TargetProcess::Other;
    }

    PCWSTR fileName = wcsrchr(modulePath, L'\\');
    fileName = fileName ? fileName + 1 : modulePath;

    if (_wcsicmp(fileName, L"explorer.exe") == 0) {
        return TargetProcess::Explorer;
    }

    if (_wcsicmp(fileName, L"ShellHost.exe") == 0) {
        return TargetProcess::ShellHost;
    }

    return TargetProcess::Other;
}

PCWSTR TargetProcessToString(TargetProcess targetProcess) {
    switch (targetProcess) {
        case TargetProcess::Explorer:
            return L"explorer";
        case TargetProcess::ShellHost:
            return L"ShellHost";
        default:
            return L"other";
    }
}

bool IsExplorerTarget() {
    return g_targetProcess == TargetProcess::Explorer;
}

HWND FindCurrentProcessTaskbarWnd() {
    HWND taskbarWnd = nullptr;

    EnumWindows(
        [](HWND hWnd, LPARAM lParam) -> BOOL {
            DWORD processId = 0;
            wchar_t className[32] = {};

            if (GetWindowThreadProcessId(hWnd, &processId) &&
                processId == GetCurrentProcessId() &&
                GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
                _wcsicmp(className, L"Shell_TrayWnd") == 0) {
                *reinterpret_cast<HWND*>(lParam) = hWnd;

                return FALSE;
            }

            return TRUE;
        },
        reinterpret_cast<LPARAM>(&taskbarWnd)
    );

    return taskbarWnd;
}

std::wstring Trim(std::wstring_view value) {
    size_t start = 0;

    while (start < value.size() && iswspace(value[start])) {
        start++;
    }

    size_t end = value.size();

    while (end > start && iswspace(value[end - 1])) {
        end--;
    }

    return std::wstring(value.substr(start, end - start));
}

std::vector<int> ParseMonitorListInOrder(std::wstring_view value) {
    std::vector<int> result;
    std::set<int> seen;

    auto flushToken = [&result, &seen](std::wstring_view tokenView) {
        std::wstring token = Trim(tokenView);

        if (token.empty()) {
            return;
        }

        int monitorNumber = _wtoi(token.c_str());

        if (monitorNumber > 0 && seen.insert(monitorNumber).second) {
            result.push_back(monitorNumber);
        }
    };

    size_t tokenStart = 0;

    while (tokenStart <= value.size()) {
        size_t commaPos = value.find(L',', tokenStart);

        if (commaPos == std::wstring_view::npos) {
            flushToken(value.substr(tokenStart));

            break;
        }

        flushToken(value.substr(tokenStart, commaPos - tokenStart));
        tokenStart = commaPos + 1;
    }

    return result;
}

std::wstring ReadStringSetting(PCWSTR settingName, PCWSTR defaultValue) {
    PCWSTR value = Wh_GetStringSetting(settingName);
    std::wstring result = value ? value : defaultValue;

    if (value) {
        Wh_FreeStringSetting(value);
    }

    return result;
}

void LoadSettings() {
    std::wstring monitorMode = ReadStringSetting(L"monitorMode", L"all");
    g_settings.monitorMode = _wcsicmp(monitorMode.c_str(), L"selected") == 0 ? MonitorMode::Selected : MonitorMode::All;

    std::wstring selectedMonitors = ReadStringSetting(L"selectedMonitors", L"1");
    g_settings.selectedMonitorOrder = ParseMonitorListInOrder(selectedMonitors);
    g_settings.selectedMonitors = std::set<int>(
        g_settings.selectedMonitorOrder.begin(),
        g_settings.selectedMonitorOrder.end()
    );

    std::wstring components = ReadStringSetting(L"components", L"all");

    if (_wcsicmp(components.c_str(), L"tray") == 0) {
        g_settings.components = Components::Tray;
    } else if (_wcsicmp(components.c_str(), L"controlCenter") == 0) {
        g_settings.components = Components::ControlCenter;
    } else {
        g_settings.components = Components::All;
    }

    g_settings.enableVerboseLogging = Wh_GetIntSetting(L"enableVerboseLogging") != 0;
    g_settings.enableTreeDump = Wh_GetIntSetting(L"enableTreeDump") != 0;

    Wh_Log_safe(
        L"taskbar-multi-tray : settings monitorMode=%s selectedMonitors=%s "
        L"components=%s verbose=%d treeDump=%d",
        MonitorModeToString(g_settings.monitorMode),
        selectedMonitors.c_str(), ComponentsToString(g_settings.components),
        g_settings.enableVerboseLogging, g_settings.enableTreeDump
    );
}

struct MonitorEntry {
    HMONITOR monitor;
    int oneBasedIndex;
};

std::vector<MonitorEntry> EnumerateMonitors() {
    std::vector<MonitorEntry> monitors;
    int nextIndex = 1;
    struct EnumContext {
        std::vector<MonitorEntry>* monitors;
        int* nextIndex;
    } context{&monitors, &nextIndex};

    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
            auto* context = reinterpret_cast<EnumContext*>(lParam);
            context -> monitors -> push_back({monitor, *context -> nextIndex});
            (*context -> nextIndex)++;
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&context)
    );

    return monitors;
}

HMONITOR GetMonitorByIndex(int monitorIndex) {
    for (const auto& entry : EnumerateMonitors()) {
        if (entry.oneBasedIndex == monitorIndex) {
            return entry.monitor;
        }
    }

    return nullptr;
}

int GetMonitorIndex(HMONITOR monitor) {
    if (!monitor) {
        return 0;
    }

    for (const auto& entry : EnumerateMonitors()) {
        if (entry.monitor == monitor) {
            return entry.oneBasedIndex;
        }
    }

    return 0;
}

HMONITOR GetActualMonitorFromWindow(HWND hWnd, DWORD flags) {
    if (MonitorFromWindow_Original) {
        return MonitorFromWindow_Original(hWnd, flags);
    }

    return MonitorFromWindow(hWnd, flags);
}

HMONITOR GetActualMonitorFromPoint(POINT point, DWORD flags) {
    if (MonitorFromPoint_Original) {
        return MonitorFromPoint_Original(point, flags);
    }

    return MonitorFromPoint(point, flags);
}

HMONITOR GetActualMonitorFromRect(LPCRECT rect, DWORD flags) {
    if (MonitorFromRect_Original) {
        return MonitorFromRect_Original(rect, flags);
    }

    return MonitorFromRect(rect, flags);
}

int GetMonitorIndexForWindow(HWND hWnd) {
    HMONITOR windowMonitor = GetActualMonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    return GetMonitorIndex(windowMonitor);
}

bool ShouldApplyToTaskbar(HWND hWnd) {
    if (g_settings.monitorMode == MonitorMode::All) {
        return true;
    }

    int monitorIndex = GetMonitorIndexForWindow(hWnd);

    return monitorIndex > 0 && g_settings.selectedMonitors.count(monitorIndex) != 0;
}

bool ShouldApplyToMonitor(HMONITOR monitor) {
    if (g_settings.monitorMode == MonitorMode::All) {
        return true;
    }

    int monitorIndex = GetMonitorIndex(monitor);

    return monitorIndex > 0 && g_settings.selectedMonitors.count(monitorIndex) != 0;
}

void* GetSharedPtrObject(void* sharedPtr) {
    if (!sharedPtr) {
        return nullptr;
    }

    return reinterpret_cast<void**>(sharedPtr)[0];
}

void* GetSharedPtrControlBlock(void* sharedPtr) {
    if (!sharedPtr) {
        return nullptr;
    }

    return reinterpret_cast<void**>(sharedPtr)[1];
}

bool WantsTray() {
    return g_settings.components == Components::All || g_settings.components == Components::Tray;
}

bool WantsControlCenter() {
    return g_settings.components == Components::All || g_settings.components == Components::ControlCenter;
}

bool WantsNotificationCenter() {
    // On Windows 11 this is also the date/time surface, which Windows already exposes on secondary taskbars. Keep it available even in tray-only mode.
    return true;
}

HMONITOR GetSingleSelectedMonitorForPrimaryTray() {
    if (g_settings.monitorMode != MonitorMode::Selected ||
        g_settings.selectedMonitorOrder.empty()) {

            return nullptr;
    }

    int monitorIndex = g_settings.selectedMonitorOrder.front();
    HMONITOR monitor = GetMonitorByIndex(monitorIndex);

    if (!monitor) {
        Wh_Log_safe(L"taskbar-multi-tray : selected monitor %d not found", monitorIndex);
    }

    return monitor;
}

HMONITOR GetRealPrimaryTrayTargetMonitor() {
    if (g_restoringNativeTaskbars) {
        return g_nativePrimaryRestoreMonitor;
    }

    if (g_runtimePrimaryTrayMonitor) {
        return g_runtimePrimaryTrayMonitor;
    }

    if (g_sharedProxyState.runtimePrimaryTrayMonitor) {
        return g_sharedProxyState.runtimePrimaryTrayMonitor;
    }

    return GetSingleSelectedMonitorForPrimaryTray();
}

HMONITOR GetActiveProxyFlyoutMonitor();

HMONITOR GetPrimaryMonitorQueryTarget() {
    if (!IsActiveFlyoutRedirectionSuppressed()) {
        if (HMONITOR proxyMonitor = GetActiveProxyFlyoutMonitor()) {
            return proxyMonitor;
        }
    }

    if (g_secondaryInitMonitor) {
        return g_secondaryInitMonitor;
    }

    return GetRealPrimaryTrayTargetMonitor();
}

HMONITOR GetActiveProxyFlyoutMonitor() {
    HMONITOR proxyMonitor = g_proxyFlyoutMonitor
        ? g_proxyFlyoutMonitor
        : g_sharedProxyState.proxyFlyoutMonitor;
    DWORD proxyUntilTick = g_proxyFlyoutMonitor
        ? g_proxyFlyoutUntilTick
        : g_sharedProxyState.proxyFlyoutUntilTick;

    if (g_proxyFlyoutMonitor && g_proxyFlyoutGeneration == g_sharedProxyState.proxyFlyoutGeneration) {
        if (!g_sharedProxyState.proxyFlyoutMonitor || !g_sharedProxyState.proxyFlyoutUntilTick) {
            // Explorer cleared the shared context. Drop the process-local copy too, otherwise ShellHost can keep redirecting after unload or after the next monitor click starts publishing a new context.
            proxyUntilTick = 0;
        } else if (static_cast<LONG>(g_sharedProxyState.proxyFlyoutUntilTick - proxyUntilTick) < 0) {
            proxyUntilTick = g_sharedProxyState.proxyFlyoutUntilTick;
            g_proxyFlyoutUntilTick = proxyUntilTick;
        }
    }

    if (!proxyMonitor) {
        return nullptr;
    }

    if (static_cast<LONG>(proxyUntilTick - GetTickCount()) <= 0) {
        g_proxyFlyoutMonitor = nullptr;
        g_proxyFlyoutMonitorIndex = 0;
        g_proxyFlyoutUntilTick = 0;
        g_proxyFlyoutGeneration = 0;
        g_proxyFlyoutKind = 0;
        g_proxyFlyoutTaskbarWnd = nullptr;
        g_proxyFlyoutAnchorValid = false;
        g_proxyFlyoutAnchorPoint = {};
        g_proxyFlyoutTaskbarRectValid = false;
        g_proxyFlyoutTaskbarRect = {};
        g_proxyFlyoutTargetDpi = 0;
        g_sharedProxyState.proxyFlyoutMonitor = nullptr;
        g_sharedProxyState.proxyFlyoutMonitorIndex = 0;
        g_sharedProxyState.proxyFlyoutUntilTick = 0;
        g_sharedProxyState.proxyFlyoutKind = 0;
        g_sharedProxyState.proxyFlyoutTaskbarWnd = 0;
        g_sharedProxyState.proxyFlyoutAnchorValid = 0;
        g_sharedProxyState.proxyFlyoutAnchorX = 0;
        g_sharedProxyState.proxyFlyoutAnchorY = 0;
        g_sharedProxyState.proxyFlyoutTaskbarRectValid = 0;
        g_sharedProxyState.proxyFlyoutTaskbarLeft = 0;
        g_sharedProxyState.proxyFlyoutTaskbarTop = 0;
        g_sharedProxyState.proxyFlyoutTaskbarRight = 0;
        g_sharedProxyState.proxyFlyoutTaskbarBottom = 0;
        g_sharedProxyState.proxyFlyoutTargetDpi = 0;

        return nullptr;
    }

    if (!g_proxyFlyoutMonitor) {
        g_proxyFlyoutMonitorIndex = g_sharedProxyState.proxyFlyoutMonitorIndex;
        g_proxyFlyoutGeneration = g_sharedProxyState.proxyFlyoutGeneration;
        g_proxyFlyoutTaskbarWnd = reinterpret_cast<HWND>(g_sharedProxyState.proxyFlyoutTaskbarWnd);
    }

    return proxyMonitor;
}

LONG GetActiveProxyFlyoutGeneration() {
    if (!GetActiveProxyFlyoutMonitor()) {
        return 0;
    }

    return g_proxyFlyoutMonitor
        ? g_proxyFlyoutGeneration
        : g_sharedProxyState.proxyFlyoutGeneration;
}

int GetActiveProxyFlyoutKind() {
    if (!GetActiveProxyFlyoutMonitor()) {
        return 0;
    }

    return g_proxyFlyoutKind
        ? g_proxyFlyoutKind
        : g_sharedProxyState.proxyFlyoutKind;
}

HWND GetActiveProxyFlyoutTaskbarWnd() {
    if (!GetActiveProxyFlyoutMonitor()) {
        return nullptr;
    }

    return g_proxyFlyoutTaskbarWnd 
        ? g_proxyFlyoutTaskbarWnd
        : reinterpret_cast<HWND>(g_sharedProxyState.proxyFlyoutTaskbarWnd);
}

bool IsKnownFlyoutKind(int kind) {
    return kind == kProxyControlCenter || kind == kProxyNotificationCenter || kind == kNativeControlCenter || kind == kNativeHiddenTray;
}

bool HasActiveKnownFlyoutContext() {
    return GetActiveProxyFlyoutMonitor() && IsKnownFlyoutKind(GetActiveProxyFlyoutKind());
}

bool ShouldRedirectPrimaryMonitorQueriesWithoutActiveFlyout() {
    // ShellHost also backs unrelated surfaces such as Windows Search. Keep the primary-display spoof out of ShellHost unless a tray/action flyout click explicitly armed the shared monitor context.
    return g_targetProcess != TargetProcess::ShellHost;
}

DWORD GetActiveProxyFlyoutUntilTick() {
    return g_proxyFlyoutMonitor
        ? g_proxyFlyoutUntilTick
        : g_sharedProxyState.proxyFlyoutUntilTick;
}

bool ActiveFlyoutContextStillHasTime() {
    DWORD untilTick = GetActiveProxyFlyoutUntilTick();

    return untilTick && static_cast<LONG>(untilTick - GetTickCount()) > 0;
}

void ShortenActiveFlyoutMonitorContext(DWORD durationMs, PCWSTR reason) {
    if (!GetActiveProxyFlyoutMonitor()) {
        return;
    }

    DWORD newUntilTick = GetTickCount() + durationMs;
    DWORD oldUntilTick = g_sharedProxyState.proxyFlyoutUntilTick;

    if (oldUntilTick &&
        static_cast<LONG>(oldUntilTick - newUntilTick) <= 0) {

        return;
    }

    if (g_proxyFlyoutMonitor) {
        g_proxyFlyoutUntilTick = newUntilTick;
    }

    g_sharedProxyState.proxyFlyoutUntilTick = newUntilTick;

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : shortening active flyout monitor "
            L"context after %s until tick=%lu generation=%ld",
            reason, newUntilTick, GetActiveProxyFlyoutGeneration()
        );
    }
}

bool GetActiveProxyFlyoutAnchorPoint(POINT* point) {
    if (!GetActiveProxyFlyoutMonitor()) {
        return false;
    }

    if (g_proxyFlyoutAnchorValid) {
        *point = g_proxyFlyoutAnchorPoint;

        return true;
    }

    if (!g_sharedProxyState.proxyFlyoutAnchorValid) {
        return false;
    }

    point -> x = g_sharedProxyState.proxyFlyoutAnchorX;
    point -> y = g_sharedProxyState.proxyFlyoutAnchorY;

    return true;
}

bool GetActiveProxyFlyoutTaskbarRect(RECT* rect) {
    if (!GetActiveProxyFlyoutMonitor()) {
        return false;
    }

    if (g_proxyFlyoutTaskbarRectValid) {
        *rect = g_proxyFlyoutTaskbarRect;

        return true;
    }

    if (!g_sharedProxyState.proxyFlyoutTaskbarRectValid) {
        return false;
    }

    *rect = {
        g_sharedProxyState.proxyFlyoutTaskbarLeft,
        g_sharedProxyState.proxyFlyoutTaskbarTop,
        g_sharedProxyState.proxyFlyoutTaskbarRight,
        g_sharedProxyState.proxyFlyoutTaskbarBottom,
    };

    return true;
}

UINT GetActiveProxyFlyoutTargetDpi() {
    if (!GetActiveProxyFlyoutMonitor()) {
        return 0;
    }

    return g_proxyFlyoutTargetDpi
        ? g_proxyFlyoutTargetDpi
        : g_sharedProxyState.proxyFlyoutTargetDpi;
}

UINT GetWindowDpiOrDefault(HWND hWnd) {
    using GetDpiForWindow_t = UINT(WINAPI*)(HWND);
    static GetDpiForWindow_t getDpiForWindow = reinterpret_cast<GetDpiForWindow_t>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));

    UINT dpi = hWnd && getDpiForWindow ? getDpiForWindow(hWnd) : 0;

    return dpi ? dpi : USER_DEFAULT_SCREEN_DPI;
}

UINT GetMonitorDpiOrDefault(HMONITOR monitor) {
    using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    static HMODULE shcoreModule = LoadLibraryEx(L"shcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    static GetDpiForMonitor_t getDpiForMonitor = shcoreModule
        ? reinterpret_cast<GetDpiForMonitor_t>(GetProcAddress(shcoreModule, "GetDpiForMonitor"))
        : nullptr;

    UINT dpiX = 0;
    UINT dpiY = 0;

    if (monitor && getDpiForMonitor && SUCCEEDED(getDpiForMonitor(monitor, 0, &dpiX, &dpiY)) && dpiX) {
        return dpiX;
    }

    return USER_DEFAULT_SCREEN_DPI;
}

bool ShouldForceActiveFlyoutForPoint(POINT point, HMONITOR targetMonitor) {
    if (!targetMonitor) {
        return false;
    }

    // The shell sometimes asks for the monitor of the foreground window or previous cursor interaction after the taskbar click. Only force ambiguous origin queries and geometry that is already on the clicked monitor, forcing every point during a rapid mixed-DPI monitor switch can leak into the previous taskbar/flyout teardown path.
    if (point.x == 0 && point.y == 0) {
        return true;
    }

    HMONITOR actualMonitor = GetActualMonitorFromPoint(point, MONITOR_DEFAULTTONULL);

    if (!actualMonitor) {
        return true;
    }

    return actualMonitor == targetMonitor;
}

bool ShouldForceActiveFlyoutForRect(LPCRECT rect, HMONITOR targetMonitor) {
    if (!targetMonitor) {
        return false;
    }

    if (!rect || (rect -> left == 0 && rect -> top == 0 && rect -> right == 0 && rect -> bottom == 0)) {
        return true;
    }

    HMONITOR actualMonitor = GetActualMonitorFromRect(rect, MONITOR_DEFAULTTONULL);

    if (!actualMonitor) {
        return true;
    }

    return actualMonitor == targetMonitor;
}

bool GetMonitorCenterPoint(HMONITOR monitor, POINT* point) {
    MONITORINFO monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return false;
    }

    RECT rect = monitorInfo.rcMonitor;
    point -> x = rect.left + (rect.right - rect.left) / 2;
    point -> y = rect.top + (rect.bottom - rect.top) / 2;

    return true;
}

bool WINAPI ImmersiveMonitorHelper_ConnectToMonitor_Hook(void* pThis, HWND hWnd, POINT point) {
    HMONITOR monitor = IsActiveFlyoutRedirectionSuppressed()
        ? nullptr
        : GetActiveProxyFlyoutMonitor();

    if (!monitor) {
        return ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, hWnd, point);
    }

    POINT targetPoint = {};

    if (!GetMonitorCenterPoint(monitor, &targetPoint)) {
        return ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, hWnd, point);
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : redirecting ConnectToMonitor from "
            L"(%ld,%ld) to monitor %d at (%ld,%ld)",
            point.x, point.y, g_proxyFlyoutMonitorIndex, targetPoint.x,
            targetPoint.y
        );
    }

    return ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, hWnd, targetPoint);
}

HRESULT WINAPI
ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Hook(void* pThis) {
    auto original = [=]() {
        return ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Original(pThis);
    };

    HMONITOR monitor = IsActiveFlyoutRedirectionSuppressed()
        ? nullptr
        : GetActiveProxyFlyoutMonitor();

    if (!monitor) {
        return original();
    }

    POINT point = {};

    if (!GetMonitorCenterPoint(monitor, &point)) {
        return original();
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : forcing immersive flyout to monitor %d "
            L"at (%ld,%ld)",
            g_proxyFlyoutMonitorIndex, point.x, point.y
        );
    }

    if (!ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, nullptr, point)) {
        Wh_Log_safe(L"taskbar-multi-tray : ConnectToMonitor failed for proxy flyout");

        return original();
    }

    return S_OK;
}

HMONITOR WINAPI MonitorFromPoint_Hook(POINT pt, DWORD flags) {
    if (!IsActiveFlyoutRedirectionSuppressed()) {
        if (HMONITOR monitor = GetActiveProxyFlyoutMonitor()) {
            if (!ShouldForceActiveFlyoutForPoint(pt, monitor)) {
                HMONITOR actualMonitor = GetActualMonitorFromPoint(pt, flags);

                if (g_settings.enableVerboseLogging) {
                    Wh_Log_safe(
                        L"taskbar-multi-tray : %s not forcing "
                        L"MonitorFromPoint (%ld,%ld) because actual "
                        L"monitor is %d and active flyout monitor is %d",
                        TargetProcessToString(g_targetProcess), pt.x, pt.y,
                        GetMonitorIndex(actualMonitor),
                        GetMonitorIndex(monitor)
                    );
                }

                return actualMonitor;
            }

            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : %s forcing MonitorFromPoint "
                    L"(%ld,%ld) to active flyout monitor %d",
                    TargetProcessToString(g_targetProcess), pt.x, pt.y,
                    GetMonitorIndex(monitor)
                );
            }

            return monitor;
        }
    }

    if (pt.x == 0 && pt.y == 0 &&
        ShouldRedirectPrimaryMonitorQueriesWithoutActiveFlyout()) {
        HMONITOR monitor = GetPrimaryMonitorQueryTarget();

        if (monitor) {
            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : %s redirecting MonitorFromPoint "
                    L"(0,0) to monitor %d",
                    TargetProcessToString(g_targetProcess),
                    GetMonitorIndex(monitor)
                );
            }

            return monitor;
        }
    }

    return MonitorFromPoint_Original(pt, flags);
}

HMONITOR WINAPI MonitorFromRect_Hook(LPCRECT rect, DWORD flags) {
    if (!IsActiveFlyoutRedirectionSuppressed()) {
        if (HMONITOR monitor = GetActiveProxyFlyoutMonitor()) {
            if (!ShouldForceActiveFlyoutForRect(rect, monitor)) {
                HMONITOR actualMonitor = GetActualMonitorFromRect(rect, flags);

                if (g_settings.enableVerboseLogging) {
                    RECT loggedRect = rect ? *rect : RECT{};
                    Wh_Log_safe(
                        L"taskbar-multi-tray : %s not forcing "
                        L"MonitorFromRect (%ld,%ld,%ld,%ld) because "
                        L"actual monitor is %d and active flyout monitor "
                        L"is %d",
                        TargetProcessToString(g_targetProcess),
                        loggedRect.left, loggedRect.top, loggedRect.right,
                        loggedRect.bottom, GetMonitorIndex(actualMonitor),
                        GetMonitorIndex(monitor)
                    );
                }

                return actualMonitor;
            }

            if (g_settings.enableVerboseLogging) {
                RECT loggedRect = rect ? *rect : RECT{};
                Wh_Log_safe(
                    L"taskbar-multi-tray : %s forcing MonitorFromRect "
                    L"(%ld,%ld,%ld,%ld) to active flyout monitor %d",
                    TargetProcessToString(g_targetProcess), loggedRect.left,
                    loggedRect.top, loggedRect.right, loggedRect.bottom,
                    GetMonitorIndex(monitor)
                );
            }

            return monitor;
        }
    }

    if (rect && rect -> left == 0 && rect -> top == 0 && rect -> right == 0 &&
        rect -> bottom == 0 &&
        ShouldRedirectPrimaryMonitorQueriesWithoutActiveFlyout()) {
        HMONITOR monitor = GetPrimaryMonitorQueryTarget();

        if (monitor) {
            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : %s redirecting MonitorFromRect "
                    L"(0,0,0,0) to monitor %d",
                    TargetProcessToString(g_targetProcess),
                    GetMonitorIndex(monitor)
                );
            }

            return monitor;
        }
    }

    return MonitorFromRect_Original(rect, flags);
}

HMONITOR WINAPI MonitorFromWindow_Hook(HWND hWnd, DWORD flags) {
    if (!IsActiveFlyoutRedirectionSuppressed()) {
        if (HMONITOR monitor = GetActiveProxyFlyoutMonitor()) {
            std::wstring className = hWnd ? GetWindowClassName(hWnd) : std::wstring();
            bool isNativeFlyoutPopup = className == L"TopLevelWindowForOverflowXamlIsland" || className == L"ControlCenterWindow";
            HWND activeTaskbarWnd = GetActiveProxyFlyoutTaskbarWnd();
            bool isClickedTaskbar = activeTaskbarWnd && hWnd == activeTaskbarWnd;

            if (!isNativeFlyoutPopup && !isClickedTaskbar) {
                HMONITOR actualMonitor = MonitorFromWindow_Original(hWnd, flags);

                if (g_settings.enableVerboseLogging) {
                    Wh_Log_safe(
                        L"taskbar-multi-tray : %s not forcing "
                        L"MonitorFromWindow hwnd=0x%p class=%s because it "
                        L"is not the clicked taskbar or a known flyout "
                        L"popup actual=%d active=%d clicked=0x%p",
                        TargetProcessToString(g_targetProcess), hWnd,
                        className.empty() ? L"<null>" : className.c_str(),
                        GetMonitorIndex(actualMonitor),
                        GetMonitorIndex(monitor), activeTaskbarWnd
                    );
                }

                return actualMonitor;
            }

            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : %s forcing MonitorFromWindow "
                    L"hwnd=0x%p class=%s to active flyout monitor %d "
                    L"clicked=0x%p",
                    TargetProcessToString(g_targetProcess), hWnd,
                    className.empty() ? L"<null>" : className.c_str(),
                    GetMonitorIndex(monitor), activeTaskbarWnd
                );
            }

            return monitor;
        }
    }

    return MonitorFromWindow_Original(hWnd, flags);
}

bool IsNativeFlyoutPopupWindow(HWND hWnd, std::wstring* classNameResult) {
    std::wstring className = GetWindowClassName(hWnd);

    if (classNameResult) {
        *classNameResult = className;
    }

    return className == L"TopLevelWindowForOverflowXamlIsland" || className == L"ControlCenterWindow";
}

bool GetMonitorWorkRect(HMONITOR monitor, RECT* workRect) {
    MONITORINFO monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return false;
    }

    *workRect = monitorInfo.rcWork;

    return true;
}

LONG ClampLong(LONG value, LONG minValue, LONG maxValue) {
    if (maxValue < minValue) {
        return minValue;
    }

    if (value < minValue) {
        return minValue;
    }

    if (value > maxValue) {
        return maxValue;
    }

    return value;
}

bool TranslateNativeFlyoutRectToActiveMonitor(HWND hWnd, const RECT& requestedRect, RECT* translatedRect, PCWSTR apiName) {
    if (IsActiveFlyoutRedirectionSuppressed()) {
        return false;
    }

    HMONITOR targetMonitor = GetActiveProxyFlyoutMonitor();

    if (!targetMonitor) {
        return false;
    }

    std::wstring className;

    if (!IsNativeFlyoutPopupWindow(hWnd, &className)) {
        return false;
    }

    RECT targetWork = {};

    if (!GetMonitorWorkRect(targetMonitor, &targetWork)) {
        return false;
    }

    LONG width = requestedRect.right - requestedRect.left;
    LONG height = requestedRect.bottom - requestedRect.top;

    if (width <= 0 || height <= 0) {
        return false;
    }

    HMONITOR requestedMonitor = MonitorFromRect_Original
        ? MonitorFromRect_Original(&requestedRect, MONITOR_DEFAULTTONEAREST)
        : nullptr;
    UINT sourceDpi = requestedMonitor
        ? GetMonitorDpiOrDefault(requestedMonitor)
        : GetWindowDpiOrDefault(hWnd);
    UINT targetDpi = GetActiveProxyFlyoutTargetDpi();

    if (!targetDpi) {
        targetDpi = GetMonitorDpiOrDefault(targetMonitor);
    }

    if (!targetDpi) {
        targetDpi = sourceDpi;
    }

    LONG translatedWidth = width;
    LONG translatedHeight = height;

    if (targetDpi && sourceDpi && targetDpi != sourceDpi) {
        translatedWidth = MulDiv(width, targetDpi, sourceDpi);
        translatedHeight = MulDiv(height, targetDpi, sourceDpi);
    }

    LONG translatedLeft = 0;
    LONG translatedTop = 0;
    POINT anchorPoint = {};
    RECT taskbarRect = {};
    LONG flyoutTaskbarGap = MulDiv(8, targetDpi, USER_DEFAULT_SCREEN_DPI);
    bool usedAnchor = className == L"TopLevelWindowForOverflowXamlIsland" && GetActiveProxyFlyoutKind() == kNativeHiddenTray && GetActiveProxyFlyoutAnchorPoint(&anchorPoint);

    if (usedAnchor) {
        translatedLeft = anchorPoint.x - translatedWidth / 2;
        translatedTop = targetWork.bottom - translatedHeight - flyoutTaskbarGap;
        if (GetActiveProxyFlyoutTaskbarRect(&taskbarRect) &&
            taskbarRect.bottom > taskbarRect.top &&
            taskbarRect.right > taskbarRect.left) {
            bool horizontalTaskbar = (taskbarRect.right - taskbarRect.left) >= (taskbarRect.bottom - taskbarRect.top);

            if (horizontalTaskbar) {
                bool bottomTaskbar = taskbarRect.top >= targetWork.top + (targetWork.bottom - targetWork.top) / 2;
                translatedTop = bottomTaskbar
                    ? taskbarRect.top - translatedHeight - flyoutTaskbarGap
                    : taskbarRect.bottom + flyoutTaskbarGap;
            } else {
                bool rightTaskbar = taskbarRect.left >= targetWork.left + (targetWork.right - targetWork.left) / 2;
                translatedLeft = rightTaskbar
                    ? taskbarRect.left - translatedWidth - flyoutTaskbarGap
                    : taskbarRect.right + flyoutTaskbarGap;
                translatedTop = anchorPoint.y - translatedHeight / 2;
            }
        }
    } else {
        HMONITOR sourceMonitor = requestedMonitor;

        if (!sourceMonitor || sourceMonitor == targetMonitor) {
            return false;
        }

        RECT sourceWork = {};

        if (!GetMonitorWorkRect(sourceMonitor, &sourceWork)) {
            return false;
        }

        LONG sourceOffsetLeft = requestedRect.left - sourceWork.left;
        LONG sourceOffsetTop = requestedRect.top - sourceWork.top;

        if (targetDpi && sourceDpi && targetDpi != sourceDpi) {
            sourceOffsetLeft = MulDiv(sourceOffsetLeft, targetDpi, sourceDpi);
            sourceOffsetTop = MulDiv(sourceOffsetTop, targetDpi, sourceDpi);
        }

        translatedLeft = targetWork.left + sourceOffsetLeft;
        translatedTop = targetWork.top + sourceOffsetTop;
    }

    translatedLeft = ClampLong(translatedLeft, targetWork.left, targetWork.right - translatedWidth);
    translatedTop = ClampLong(translatedTop, targetWork.top, targetWork.bottom - translatedHeight);

    *translatedRect = {
        translatedLeft,
        translatedTop,
        translatedLeft + translatedWidth,
        translatedTop + translatedHeight,
    };

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : moving %s via %s from "
            L"(%ld,%ld,%ld,%ld) to (%ld,%ld,%ld,%ld) on monitor %d "
            L"kind=%d anchor=%d (%ld,%ld) taskbar=(%ld,%ld,%ld,%ld) "
            L"dpi=%u -> %u",
            className.c_str(), apiName, requestedRect.left,
            requestedRect.top, requestedRect.right, requestedRect.bottom,
            translatedRect -> left, translatedRect -> top,
            translatedRect -> right, translatedRect -> bottom,
            GetMonitorIndex(targetMonitor), GetActiveProxyFlyoutKind(),
            usedAnchor, anchorPoint.x, anchorPoint.y, taskbarRect.left,
            taskbarRect.top, taskbarRect.right, taskbarRect.bottom,
            sourceDpi, targetDpi
        );
    }

    ShortenActiveFlyoutMonitorContext(kNativeFlyoutMonitorContextAfterPlacementMs, apiName);
    return true;
}

bool ShouldTranslateNativeFlyoutWindowPosition(UINT uFlags) {
    if (uFlags & SWP_HIDEWINDOW) {
        return false;
    }

    if ((uFlags & SWP_NOMOVE) && (uFlags & SWP_NOSIZE)) {
        return false;
    }

    return true;
}

BOOL WINAPI SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
    if (IsModUnloading()) {
        return SetWindowPos_Original(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
    }

    RECT currentRect = {};
    GetWindowRect(hWnd, &currentRect);

    LONG width = (uFlags & SWP_NOSIZE) ? currentRect.right - currentRect.left : cx;
    LONG height = (uFlags & SWP_NOSIZE) ? currentRect.bottom - currentRect.top : cy;
    RECT requestedRect = {
        (uFlags & SWP_NOMOVE) ? currentRect.left : X,
        (uFlags & SWP_NOMOVE) ? currentRect.top : Y,
        ((uFlags & SWP_NOMOVE) ? currentRect.left : X) + width,
        ((uFlags & SWP_NOMOVE) ? currentRect.top : Y) + height,
    };

    RECT translatedRect = {};
    if (ShouldTranslateNativeFlyoutWindowPosition(uFlags) && TranslateNativeFlyoutRectToActiveMonitor(hWnd, requestedRect, &translatedRect, L"SetWindowPos")) {
        uFlags &= ~SWP_NOMOVE;
        uFlags &= ~SWP_NOSIZE;
        X = translatedRect.left;
        Y = translatedRect.top;
        cx = translatedRect.right - translatedRect.left;
        cy = translatedRect.bottom - translatedRect.top;
    }

    return SetWindowPos_Original(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

BOOL WINAPI MoveWindow_Hook(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) {
    if (IsModUnloading()) {
        return MoveWindow_Original(hWnd, X, Y, nWidth, nHeight, bRepaint);
    }

    RECT requestedRect = {X, Y, X + nWidth, Y + nHeight};
    RECT translatedRect = {};

    if (TranslateNativeFlyoutRectToActiveMonitor(hWnd, requestedRect, &translatedRect, L"MoveWindow")) {
        X = translatedRect.left;
        Y = translatedRect.top;
        nWidth = translatedRect.right - translatedRect.left;
        nHeight = translatedRect.bottom - translatedRect.top;
    }

    return MoveWindow_Original(hWnd, X, Y, nWidth, nHeight, bRepaint);
}

BOOL WINAPI SetWindowPlacement_Hook(HWND hWnd, const WINDOWPLACEMENT* lpwndpl) {
    if (IsModUnloading()) {
        return SetWindowPlacement_Original(hWnd, lpwndpl);
    }

    if (!lpwndpl) {
        return SetWindowPlacement_Original(hWnd, lpwndpl);
    }

    WINDOWPLACEMENT placement = *lpwndpl;
    RECT translatedRect = {};

    if (TranslateNativeFlyoutRectToActiveMonitor(hWnd, placement.rcNormalPosition, &translatedRect, L"SetWindowPlacement")) {
        placement.rcNormalPosition = translatedRect;

        return SetWindowPlacement_Original(hWnd, &placement);
    }

    return SetWindowPlacement_Original(hWnd, lpwndpl);
}

HDWP WINAPI DeferWindowPos_Hook(HDWP hWinPosInfo, HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags) {
    if (IsModUnloading()) {
        return DeferWindowPos_Original(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
    }

    RECT currentRect = {};
    GetWindowRect(hWnd, &currentRect);

    LONG width = (uFlags & SWP_NOSIZE) ? currentRect.right - currentRect.left : cx;
    LONG height = (uFlags & SWP_NOSIZE) ? currentRect.bottom - currentRect.top : cy;
    RECT requestedRect = {
        (uFlags & SWP_NOMOVE) ? currentRect.left : x,
        (uFlags & SWP_NOMOVE) ? currentRect.top : y,
        ((uFlags & SWP_NOMOVE) ? currentRect.left : x) + width,
        ((uFlags & SWP_NOMOVE) ? currentRect.top : y) + height,
    };

    RECT translatedRect = {};

    if (ShouldTranslateNativeFlyoutWindowPosition(uFlags) && TranslateNativeFlyoutRectToActiveMonitor( hWnd, requestedRect, &translatedRect, L"DeferWindowPos")) {
        uFlags &= ~SWP_NOMOVE;
        uFlags &= ~SWP_NOSIZE;
        x = translatedRect.left;
        y = translatedRect.top;
        cx = translatedRect.right - translatedRect.left;
        cy = translatedRect.bottom - translatedRect.top;
    }

    return DeferWindowPos_Original(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
}

BOOL WINAPI EnumDisplayDevicesW_Hook(LPCWSTR deviceName, DWORD deviceIndex, PDISPLAY_DEVICEW displayDevice, DWORD flags) {
    BOOL result = EnumDisplayDevicesW_Original(deviceName, deviceIndex, displayDevice, flags);

    if (!result || !displayDevice || deviceName || IsModUnloading()) {
        return result;
    }

    if (g_targetProcess == TargetProcess::ShellHost &&
        !HasActiveKnownFlyoutContext()) {

            return result;
    }

    HMONITOR monitor = GetPrimaryMonitorQueryTarget();

    if (!monitor) {
        return result;
    }

    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return result;
    }

    if (wcscmp(displayDevice -> DeviceName, monitorInfo.szDevice) == 0) {
        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : %s marking %s as primary display",
                TargetProcessToString(g_targetProcess),
                displayDevice -> DeviceName
            );
        }

        displayDevice -> StateFlags |= DISPLAY_DEVICE_PRIMARY_DEVICE;
    } else {
        displayDevice -> StateFlags &= ~DISPLAY_DEVICE_PRIMARY_DEVICE;
    }

    return result;
}

void BroadcastShellHookDisplayChange() {
    UINT shellhookMessage = RegisterWindowMessage(L"SHELLHOOK");
    PostMessage(HWND_BROADCAST, shellhookMessage, 35, 0);
}

void NotifyTaskbarDisplayChange(HWND taskbarWnd) {
    if (!taskbarWnd) {
        return;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : notifying taskbar display change");
    }

    // Handled by CTray::_HandleDisplayChange, this makes Explorer re-run TrayUI::_SetStuckMonitor after settings change
    SendMessage(taskbarWnd, 0x5B8, 0, 0);
    BroadcastShellHookDisplayChange();
}

std::wstring GetWindowClassName(HWND hWnd) {
    wchar_t className[64] = {};

    if (!GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
        return L"<unknown>";
    }

    return className;
}

std::wstring GetMonitorDeviceName(HWND hWnd) {
    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    if (!monitor) {
        return L"<none>";
    }

    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return L"<unknown>";
    }

    return monitorInfo.szDevice;
}

int GetVisualChildCount(FrameworkElement element) {
    try {
        return Media::VisualTreeHelper::GetChildrenCount(element);
    } catch (...) {
        return -1;
    }
}

void UpdateLayoutBestEffort(FrameworkElement element, PCWSTR debugName) {
    try {
        element.UpdateLayout();
    } catch (...) {
        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : failed to update layout for %s", debugName);
        }
    }
}

void LogTaskbarWindow(HWND hWnd, PCWSTR stage) {
    if (!g_settings.enableVerboseLogging) {
        return;
    }

    RECT rect = {};
    GetWindowRect(hWnd, &rect);

    Wh_Log_safe(
        L"taskbar-multi-tray : %s hwnd=0x%p class=%s monitor=%d device=%s "
        L"rect=(%ld,%ld)-(%ld,%ld)",
        stage, hWnd, GetWindowClassName(hWnd).c_str(),
        GetMonitorIndexForWindow(hWnd), GetMonitorDeviceName(hWnd).c_str(),
        rect.left, rect.top, rect.right, rect.bottom
    );
}

void LogElementState(FrameworkElement element, PCWSTR label) {
    if (!g_settings.enableVerboseLogging) {
        return;
    }

    if (!element) {
        Wh_Log_safe(L"taskbar-multi-tray : element %s missing", label);

        return;
    }

    try {
        auto name = element.Name();
        auto className = winrt::get_class_name(element);
        Wh_Log_safe(
            L"taskbar-multi-tray : element %s class=%s name=%s "
            L"visibility=%s opacity=%.2f hitTest=%d actual=%.1fx%.1f "
            L"children=%d",
            label, className.c_str(), name.c_str(),
            VisibilityToString(element.Visibility()), element.Opacity(),
            element.IsHitTestVisible(), element.ActualWidth(),
            element.ActualHeight(), GetVisualChildCount(element)
        );
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to inspect element %s", label);
    }
}

void* InspectableAbi(winrt::Windows::Foundation::IInspectable object) {
    return object ? winrt::get_abi(object) : nullptr;
}

bool IsXamlUiElement(winrt::Windows::Foundation::IInspectable object) {
    if (!object) {
        return false;
    }

    return static_cast<bool>(object.try_as<UIElement>());
}

bool HasAnyBinding(const CachedXamlBinding& binding) {
    return binding.dataContext || binding.itemsSource || binding.content;
}

bool CaptureBindingIfPresent(FrameworkElement element, CachedXamlBinding& binding, bool includeItemsSource, bool includeContent) {
    bool changed = false;

    auto dataContext = element.DataContext();

    if (dataContext && InspectableAbi(dataContext) != InspectableAbi(binding.dataContext)) {
        binding.dataContext = dataContext;
        changed = true;
    }

    if (includeItemsSource) {
        auto itemsControl = element.try_as<Controls::ItemsControl>();
        winrt::Windows::Foundation::IInspectable itemsSource = itemsControl ? itemsControl.ItemsSource() : nullptr;

        if (itemsSource && InspectableAbi(itemsSource) != InspectableAbi(binding.itemsSource)) {
            binding.itemsSource = itemsSource;
            changed = true;
        }
    }

    if (includeContent) {
        auto contentControl = element.try_as<Controls::ContentControl>();
        winrt::Windows::Foundation::IInspectable content = contentControl ? contentControl.Content() : nullptr;

        if (content && !IsXamlUiElement(content) && InspectableAbi(content) != InspectableAbi(binding.content)) {
            binding.content = content;
            changed = true;
        }
    }

    return changed;
}

void SharePrimaryElementBindingIfUseful(FrameworkElement element, HWND taskbarWnd, PCWSTR debugName, CachedXamlBinding& binding, bool useThisElementAsSource, bool includeItemsSource, bool includeContent) {
    if (!element) {
        return;
    }

    try {
        auto dataContext = element.DataContext();
        auto itemsControl = element.try_as<Controls::ItemsControl>();
        winrt::Windows::Foundation::IInspectable itemsSource = itemsControl ? itemsControl.ItemsSource() : nullptr;
        auto contentControl = element.try_as<Controls::ContentControl>();
        winrt::Windows::Foundation::IInspectable content = contentControl ? contentControl.Content() : nullptr;

        int monitorIndex = GetMonitorIndexForWindow(taskbarWnd);

        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : %s binding monitor=%d width=%.1f "
                L"source=%d dataContext=0x%p itemsControl=%d "
                L"itemsSource=0x%p contentControl=%d content=0x%p "
                L"contentIsUiElement=%d savedDataContext=0x%p "
                L"savedItemsSource=0x%p savedContent=0x%p",
                debugName, monitorIndex, element.ActualWidth(),
                useThisElementAsSource,
                InspectableAbi(dataContext), !!itemsControl,
                InspectableAbi(itemsSource), !!contentControl,
                InspectableAbi(content), IsXamlUiElement(content),
                InspectableAbi(binding.dataContext),
                InspectableAbi(binding.itemsSource),
                InspectableAbi(binding.content)
            );
        }

        if (useThisElementAsSource) {
            if (CaptureBindingIfPresent(element, binding, includeItemsSource, includeContent) &&
                g_settings.enableVerboseLogging) {
                Wh_Log_safe(L"taskbar-multi-tray : cached primary %s binding", debugName);
            }

            return;
        }

        if (!HasAnyBinding(binding)) {
            return;
        }

        bool changed = false;

        if (binding.dataContext && InspectableAbi(dataContext) != InspectableAbi(binding.dataContext)) {
            element.DataContext(binding.dataContext);
            changed = true;
        }

        if (includeItemsSource && itemsControl && binding.itemsSource && InspectableAbi(itemsSource) != InspectableAbi(binding.itemsSource)) {
            itemsControl.ItemsSource(binding.itemsSource);
            changed = true;
        }

        if (includeContent && contentControl && binding.content && !IsXamlUiElement(binding.content) && InspectableAbi(content) != InspectableAbi(binding.content)) {
            contentControl.Content(binding.content);
            changed = true;
        }

        if (changed) {
            UpdateLayoutBestEffort(element, debugName);

            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(L"taskbar-multi-tray : applied primary %s binding to monitor %d", debugName, monitorIndex);
                LogElementState(element, debugName);
            }
        }
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to share %s binding", debugName);
    }
}

void DumpElementTree(FrameworkElement element, const std::wstring& label, int depth, int maxDepth) {
    if (!g_settings.enableVerboseLogging || !g_settings.enableTreeDump || !element) {
        return;
    }

    int childCount = GetVisualChildCount(element);
    Wh_Log_safe(L"taskbar-multi-tray : tree %s depth=%d children=%d", label.c_str(), depth, childCount);

    if (childCount <= 0) {
        return;
    }

    constexpr int kMaxChildrenToDump = 32;
    int childrenToDump = childCount < kMaxChildrenToDump ? childCount : kMaxChildrenToDump;

    for (int i = 0; i < childrenToDump; i++) {
        try {
            auto child = Media::VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();

            if (!child) {
                Wh_Log_safe(L"taskbar-multi-tray : tree %s[%d] is not a FrameworkElement", label.c_str(), i);

                continue;
            }

            auto name = child.Name();
            auto className = winrt::get_class_name(child);
            Wh_Log_safe(
                L"taskbar-multi-tray : tree %s[%d] class=%s name=%s "
                L"visibility=%s opacity=%.2f hitTest=%d actual=%.1fx%.1f "
                L"children=%d",
                label.c_str(), i, className.c_str(), name.c_str(),
                VisibilityToString(child.Visibility()), child.Opacity(),
                child.IsHitTestVisible(), child.ActualWidth(),
                child.ActualHeight(), GetVisualChildCount(child)
            );

            if (depth < maxDepth) {
                DumpElementTree(child, label + L"/" + std::to_wstring(i), depth + 1, maxDepth);
            }
        } catch (...) {
            Wh_Log_safe(L"taskbar-multi-tray : failed to inspect tree %s[%d]", label.c_str(), i);
        }
    }

    if (childCount > kMaxChildrenToDump) {
        Wh_Log_safe(L"taskbar-multi-tray : tree %s truncated after %d of %d children", label.c_str(), kMaxChildrenToDump, childCount);
    }
}

void DumpElementChildren(FrameworkElement element, PCWSTR label) {
    DumpElementTree(element, label, 0, 2);
}

void ForceVisibleDescendants(FrameworkElement element, const std::wstring& label, int depth, int maxDepth) {
    if (!element || depth > maxDepth) {
        return;
    }

    int childCount = GetVisualChildCount(element);

    if (childCount <= 0) {
        return;
    }

    constexpr int kMaxChildrenToVisit = 16;

    int childrenToVisit = childCount < kMaxChildrenToVisit ? childCount : kMaxChildrenToVisit;

    for (int i = 0; i < childrenToVisit; i++) {
        try {
            auto child = Media::VisualTreeHelper::GetChild(element, i) .try_as<FrameworkElement>();

            if (!child) {
                continue;
            }

            std::wstring childLabel = label + L"/" + std::to_wstring(i);

            if (g_settings.enableVerboseLogging) {
                auto name = child.Name();
                auto className = winrt::get_class_name(child);
                Wh_Log_safe(
                    L"taskbar-multi-tray : hidden-stack child %s "
                    L"class=%s name=%s visibility=%s opacity=%.2f "
                    L"hitTest=%d actual=%.1fx%.1f children=%d",
                    childLabel.c_str(), className.c_str(), name.c_str(),
                    VisibilityToString(child.Visibility()), child.Opacity(),
                    child.IsHitTestVisible(), child.ActualWidth(),
                    child.ActualHeight(), GetVisualChildCount(child)
                );
            }

            child.Visibility(Visibility::Visible);
            child.Opacity(1.0);
            child.IsHitTestVisible(true);

            ForceVisibleDescendants(child, childLabel, depth + 1, maxDepth);
        } catch (...) {
            Wh_Log_safe(L"taskbar-multi-tray : failed to force hidden-stack child %s/%d", label.c_str(), i);
        }
    }
}

FrameworkElement EnumChildElements(FrameworkElement element, const std::function<bool(FrameworkElement)>& enumCallback) {
    int childCount = Media::VisualTreeHelper::GetChildrenCount(element);

    for (int i = 0; i < childCount; i++) {
        auto child = Media::VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();

        if (!child) {
            continue;
        }

        if (enumCallback(child)) {
            return child;
        }
    }

    return nullptr;
}

FrameworkElement FirstChildElement(FrameworkElement element) {
    if (!element) {
        return nullptr;
    }

    try {
        int childCount = Media::VisualTreeHelper::GetChildrenCount(element);

        if (childCount <= 0) {
            return nullptr;
        }

        return Media::VisualTreeHelper::GetChild(element, 0).try_as<FrameworkElement>();
    } catch (...) {
        return nullptr;
    }
}

FrameworkElement FindChildByName(FrameworkElement element, PCWSTR name) {
    return EnumChildElements(element, [name](FrameworkElement child) {
        return child.Name() == name;
    });
}

FrameworkElement FindChildByClassName(FrameworkElement element, PCWSTR className) {
    return EnumChildElements(element, [className](FrameworkElement child) {
        return winrt::get_class_name(child) == className;
    });
}

FrameworkElement FindDescendantByClassName(FrameworkElement element, PCWSTR className, int maxDepth = 6) {
    if (!element || maxDepth < 0) {
        return nullptr;
    }

    try {
        int childCount = Media::VisualTreeHelper::GetChildrenCount(element);

        for (int i = 0; i < childCount; i++) {
            auto child = Media::VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();

            if (!child) {
                continue;
            }

            if (winrt::get_class_name(child) == className) {
                return child;
            }

            if (FrameworkElement descendant = FindDescendantByClassName(child, className, maxDepth - 1)) {
                return descendant;
            }
        }
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to find descendant class %s", className);
    }

    return nullptr;
}

void ResetExplicitWidth(FrameworkElement element) {
    element.Width(std::numeric_limits<double>::quiet_NaN());
}

bool ForceVisible(FrameworkElement element, PCWSTR debugName) {
    if (!element) {
        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : %s missing, can't force visible", debugName);
        }

        return false;
    }

    LogElementState(element, debugName);

    try {
        element.Visibility(Visibility::Visible);
        element.Opacity(1.0);
        element.IsHitTestVisible(true);
        ResetExplicitWidth(element);
        UpdateLayoutBestEffort(element, debugName);

        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : made %s visible", debugName);
            LogElementState(element, debugName);
        }

        return true;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to update %s", debugName);

        return false;
    }
}

bool SetElementVisibility(FrameworkElement element, PCWSTR debugName, bool visible) {
    if (!element) {
        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : %s missing, can't set visibility", debugName);
        }

        return false;
    }

    LogElementState(element, debugName);

    try {
        element.Visibility(visible ? Visibility::Visible : Visibility::Collapsed);
        element.Opacity(1.0);
        element.IsHitTestVisible(visible);

        if (visible) {
            ResetExplicitWidth(element);
        } else {
            element.MinWidth(0.0);
            element.Width(0.0);
        }

        UpdateLayoutBestEffort(element, debugName);

        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : made %s %s", debugName, visible ? L"visible" : L"collapsed");
            LogElementState(element, debugName);
        }

        return true;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to set visibility for %s", debugName);
        return false;
    }
}

bool ForceVisibleWithMinWidth(FrameworkElement element, PCWSTR debugName, double minWidth, bool setExactWidthWhenCollapsed = false) {
    if (!ForceVisible(element, debugName)) {
        return false;
    }

    try {
        bool changed = false;

        if (element.MinWidth() < minWidth) {
            element.MinWidth(minWidth);
            changed = true;
        }

        double width = element.Width();

        if (width == width && width < minWidth) {
            if (setExactWidthWhenCollapsed) {
                element.Width(minWidth);
            } else {
                ResetExplicitWidth(element);
            }

            changed = true;
        } else if (setExactWidthWhenCollapsed && element.ActualWidth() < minWidth && (width != width || width < minWidth)) {
            element.Width(minWidth);
            changed = true;
        }

        if (changed) {
            UpdateLayoutBestEffort(element, debugName);

            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : forced %s minWidth=%.1f "
                    L"exactWidth=%d",
                    debugName, minWidth, setExactWidthWhenCollapsed
                );
                LogElementState(element, debugName);
            }
        }

        if (g_settings.enableVerboseLogging && element.ActualWidth() < 4.0) {
            Wh_Log_safe(
                L"taskbar-multi-tray : %s is still width-collapsed; the "
                L"XAML element exists but Windows didn't populate/layout "
                L"usable content here",
                debugName
            );
        }

        return true;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to size %s", debugName);

        return true;
    }
}

void ApplyFrameMinWidth(FrameworkElement element, PCWSTR debugName, double minWidth) {
    if (!element) {
        return;
    }

    try {
        if (minWidth <= 0) {
            element.MinWidth(0.0);
            ResetExplicitWidth(element);
            UpdateLayoutBestEffort(element, debugName);

            return;
        }

        if (element.MinWidth() != minWidth) {
            element.MinWidth(minWidth);
            UpdateLayoutBestEffort(element, debugName);

            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(L"taskbar-multi-tray : forced %s minWidth=%.1f", debugName, minWidth);
                LogElementState(element, debugName);
            }
        }
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to size %s", debugName);
    }
}

bool IsPrimaryTaskbarWindow(HWND hWnd) {
    wchar_t className[32] = {};

    return GetClassNameW(hWnd, className, ARRAYSIZE(className)) && _wcsicmp(className, L"Shell_TrayWnd") == 0;
}

bool IsRealPrimaryTrayTargetWindow(HWND hWnd) {
    HMONITOR targetMonitor = GetRealPrimaryTrayTargetMonitor();

    if (targetMonitor) {
        return MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST) == targetMonitor;
    }

    return IsPrimaryTaskbarWindow(hWnd);
}

bool IsCurrentPrimaryTrayBindingSource(HWND taskbarWnd, FrameworkElement notificationAreaIcons) {
    if (!notificationAreaIcons || notificationAreaIcons.ActualWidth() < 4.0) {
        return false;
    }

    return IsRealPrimaryTrayTargetWindow(taskbarWnd);
}

bool ApplyNonTargetStyle(XamlRoot xamlRoot, HWND taskbarWnd) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : resetting skipped monitor %d to "
            L"default secondary tray state",
            GetMonitorIndexForWindow(taskbarWnd)
        );
    }

    FrameworkElement child = xamlRoot.Content().try_as<FrameworkElement>();

    if (!child) {
        Wh_Log_safe(L"taskbar-multi-tray : XamlRoot content is not FrameworkElement");

        return false;
    }

    child = FindChildByClassName(child, L"SystemTray.SystemTrayFrame");

    if (!child) {
        int monitorIndex = GetMonitorIndexForWindow(taskbarWnd);
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrame not found for monitor %d", monitorIndex);

        return false;
    }

    FrameworkElement systemTrayFrameGrid = FindChildByName(child, L"SystemTrayFrameGrid");

    if (!systemTrayFrameGrid) {
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrameGrid not found");

        return false;
    }

    ApplyFrameMinWidth(child, L"SystemTrayFrame", 78.0);
    ApplyFrameMinWidth(systemTrayFrameGrid, L"SystemTrayFrameGrid", 78.0);

    bool applied = false;
    applied |= SetElementVisibility(FindChildByName(systemTrayFrameGrid, L"NotifyIconStack"), L"NotifyIconStack", false);
    applied |= SetElementVisibility(FindChildByName(systemTrayFrameGrid, L"NotificationAreaIcons"), L"NotificationAreaIcons", false);
    applied |= SetElementVisibility(FindChildByName(systemTrayFrameGrid, L"ControlCenterButton"), L"ControlCenterButton", false);
    applied |= ForceVisibleWithMinWidth(FindChildByName(systemTrayFrameGrid, L"NotificationCenterButton"), L"NotificationCenterButton", 78.0, true);

    return applied;
}

bool ApplyNativePrimaryStyle(XamlRoot xamlRoot, HWND taskbarWnd) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : restoring native primary tray state on "
            L"monitor %d",
            GetMonitorIndexForWindow(taskbarWnd)
        );
    }

    FrameworkElement child = xamlRoot.Content().try_as<FrameworkElement>();

    if (!child) {
        Wh_Log_safe(L"taskbar-multi-tray : XamlRoot content is not FrameworkElement");

        return false;
    }

    child = FindChildByClassName(child, L"SystemTray.SystemTrayFrame");

    if (!child) {
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrame not found while restoring native primary state");

        return false;
    }

    FrameworkElement systemTrayFrameGrid = FindChildByName(child, L"SystemTrayFrameGrid");
    if (!systemTrayFrameGrid) {
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrameGrid not found while restoring native primary state");

        return false;
    }

    ApplyFrameMinWidth(child, L"SystemTrayFrame", 0.0);
    ApplyFrameMinWidth(systemTrayFrameGrid, L"SystemTrayFrameGrid", 0.0);

    bool applied = false;
    FrameworkElement notifyIconStack = FindChildByName(systemTrayFrameGrid, L"NotifyIconStack");
    applied |= ForceVisible(notifyIconStack, L"NotifyIconStack");
    applied |= ForceVisible(FirstChildElement(notifyIconStack), L"NotifyIconStackChild");
    applied |= ForceVisible(FindDescendantByClassName(notifyIconStack, L"SystemTray.StackListView"), L"NotifyIconStackListView");
    applied |= ForceVisible(FindChildByName(systemTrayFrameGrid, L"NotificationAreaIcons"), L"NotificationAreaIcons");
    applied |= ForceVisible(FindChildByName(systemTrayFrameGrid, L"ControlCenterButton"), L"ControlCenterButton");
    applied |= ForceVisible(FindChildByName(systemTrayFrameGrid, L"NotificationCenterButton"), L"NotificationCenterButton");

    return applied;
}

bool ApplyStyle(XamlRoot xamlRoot, HWND taskbarWnd) {
    LogTaskbarWindow(taskbarWnd, L"ApplyStyle");

    if (!ShouldApplyToTaskbar(taskbarWnd)) {
        return ApplyNonTargetStyle(xamlRoot, taskbarWnd);
    }

    FrameworkElement child = xamlRoot.Content().try_as<FrameworkElement>();

    if (!child) {
        Wh_Log_safe(L"taskbar-multi-tray : XamlRoot content is not FrameworkElement");

        return false;
    }

    LogElementState(child, L"XamlRoot.Content");

    child = FindChildByClassName(child, L"SystemTray.SystemTrayFrame");

    if (!child) {
        int monitorIndex = GetMonitorIndexForWindow(taskbarWnd);
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrame not found for monitor %d", monitorIndex);

        return false;
    }

    ForceVisible(child, L"SystemTrayFrame");

    FrameworkElement systemTrayFrameGrid = FindChildByName(child, L"SystemTrayFrameGrid");
    if (!systemTrayFrameGrid) {
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrameGrid not found");

        return false;
    }

    ForceVisible(systemTrayFrameGrid, L"SystemTrayFrameGrid");

    if (g_settings.enableTreeDump) {
        DumpElementChildren(systemTrayFrameGrid, L"SystemTrayFrameGrid");
    }

    double frameMinWidth = 0.0;

    if (WantsTray()) {
        frameMinWidth += 32.0;
    }

    if (WantsControlCenter()) {
        frameMinWidth += 117.0;
    }

    if (WantsNotificationCenter()) {
        frameMinWidth += 78.0;
    }

    ApplyFrameMinWidth(child, L"SystemTrayFrame", frameMinWidth);
    ApplyFrameMinWidth(systemTrayFrameGrid, L"SystemTrayFrameGrid", frameMinWidth);

    bool applied = false;
    bool useThisTaskbarAsPrimaryTrayBindingSource = IsRealPrimaryTrayTargetWindow(taskbarWnd);

    if (WantsTray()) {
        FrameworkElement notifyIconStack = FindChildByName(systemTrayFrameGrid, L"NotifyIconStack");
        applied |= ForceVisibleWithMinWidth(notifyIconStack, L"NotifyIconStack", 32.0, false);
        FrameworkElement notifyIconStackChild = FirstChildElement(notifyIconStack);
        applied |= ForceVisibleWithMinWidth(notifyIconStackChild, L"NotifyIconStackChild", 32.0, false);
        FrameworkElement notifyIconStackListView = FindDescendantByClassName(notifyIconStack, L"SystemTray.StackListView");
        applied |= ForceVisibleWithMinWidth(notifyIconStackListView, L"NotifyIconStackListView", 32.0, false);
        ForceVisibleDescendants(notifyIconStack, L"NotifyIconStack", 0, 6);
        UpdateLayoutBestEffort(notifyIconStack, L"NotifyIconStack");
        FrameworkElement notificationAreaIcons = FindChildByName(systemTrayFrameGrid, L"NotificationAreaIcons");
        useThisTaskbarAsPrimaryTrayBindingSource = IsCurrentPrimaryTrayBindingSource(taskbarWnd, notificationAreaIcons);
        applied |= ForceVisible(notificationAreaIcons, L"NotificationAreaIcons");
        SharePrimaryElementBindingIfUseful(notificationAreaIcons, taskbarWnd, L"NotificationAreaIcons", g_primaryNotificationAreaIconsBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, false);
        SharePrimaryElementBindingIfUseful(notifyIconStack, taskbarWnd, L"NotifyIconStack", g_primaryNotifyIconStackBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, true);
        SharePrimaryElementBindingIfUseful(notifyIconStackChild, taskbarWnd, L"NotifyIconStackChild", g_primaryNotifyIconStackChildBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, true);
        SharePrimaryElementBindingIfUseful(notifyIconStackListView, taskbarWnd, L"NotifyIconStackListView", g_primaryNotifyIconStackListViewBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, false);

        try {
            double notificationAreaIconsWidth = notificationAreaIcons ? notificationAreaIcons.ActualWidth() : 0.0;
            SetCachedNotificationAreaIconsWidth(taskbarWnd, notificationAreaIconsWidth);

            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : cached promoted icon width "
                    L"monitor=%d width=%.1f",
                    GetMonitorIndexForWindow(taskbarWnd),
                    notificationAreaIconsWidth
                );
            }
        } catch (...) {
            SetCachedNotificationAreaIconsWidth(taskbarWnd, 0.0);
        }
    } else {
        SetCachedNotificationAreaIconsWidth(taskbarWnd, 0.0);
        applied |= SetElementVisibility(FindChildByName(systemTrayFrameGrid, L"NotifyIconStack"), L"NotifyIconStack", false);
        applied |= SetElementVisibility(FindChildByName(systemTrayFrameGrid, L"NotificationAreaIcons"), L"NotificationAreaIcons", false);
    }

    if (WantsControlCenter()) {
        FrameworkElement controlCenterButton = FindChildByName(systemTrayFrameGrid, L"ControlCenterButton");
        applied |= ForceVisibleWithMinWidth(controlCenterButton, L"ControlCenterButton", 117.0, true);
        SharePrimaryElementBindingIfUseful(controlCenterButton, taskbarWnd, L"ControlCenterButton", g_primaryControlCenterButtonBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, true);
    } else {
        applied |= SetElementVisibility(FindChildByName(systemTrayFrameGrid, L"ControlCenterButton"), L"ControlCenterButton", false);
    }

    if (WantsNotificationCenter()) {
        applied |= ForceVisibleWithMinWidth(FindChildByName(systemTrayFrameGrid, L"NotificationCenterButton"), L"NotificationCenterButton", 78.0, true);
    } else {
        applied |= SetElementVisibility(FindChildByName(systemTrayFrameGrid, L"NotificationCenterButton"), L"NotificationCenterButton", false);
    }

    return applied;
}

constexpr UINT_PTR kTaskbarSubclassId = 1;
constexpr int kShowDesktopWidth = 12;
constexpr int kNotificationCenterWidth = 78;
constexpr int kControlCenterWidth = 117;
constexpr int kNotifyIconStackWidth = 32;
constexpr int kNativeFlyoutHitSlop = 8;
constexpr UINT_PTR kRestoreProxyTrayTimerId = 2;
constexpr UINT_PTR kClearNativeFlyoutMonitorTimerId = 3;
constexpr UINT_PTR kDeferredApplySettingsTimerId = 4;

void ApplySettingsFromTaskbarThread(void*);

int ScaleTaskbarMetricForWindow(HWND hWnd, int value) {
    return MulDiv(value, GetWindowDpiOrDefault(hWnd), USER_DEFAULT_SCREEN_DPI);
}

int ScaleTaskbarMetricForWindow(HWND hWnd, double value) {
    return MulDiv(static_cast<int>(value + 0.5), GetWindowDpiOrDefault(hWnd), USER_DEFAULT_SCREEN_DPI);
}

UINT ProxyOpenMessage() {
    static const UINT message = RegisterWindowMessage(L"Windhawk_ProxyOpenFlyout_" WH_MOD_ID);

    return message;
}

bool IsSecondaryTaskbarWindow(HWND hWnd) {
    wchar_t className[32] = {};

    return GetClassNameW(hWnd, className, ARRAYSIZE(className)) && _wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0;
}

WPARAM HitTestProxyFlyout(HWND hWnd, LPARAM lParam) {
    if (!IsSecondaryTaskbarWindow(hWnd) || !ShouldApplyToTaskbar(hWnd)) {
        return 0;
    }

    RECT clientRect = {};

    if (!GetClientRect(hWnd, &clientRect)) {
        return 0;
    }

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    if (width <= height) {
        return 0;
    }

    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    if (x < clientRect.left || x >= clientRect.right || y < clientRect.top || y >= clientRect.bottom) {
        return 0;
    }

    int xFromRight = clientRect.right - x;
    int hitSlop = ScaleTaskbarMetricForWindow(hWnd, kNativeFlyoutHitSlop);
    int notificationStart = ScaleTaskbarMetricForWindow(hWnd, kShowDesktopWidth);
    int notificationEnd = notificationStart + ScaleTaskbarMetricForWindow( hWnd, kNotificationCenterWidth);
    int controlStart = notificationEnd;
    int controlEnd = controlStart + ScaleTaskbarMetricForWindow(hWnd, kControlCenterWidth);
    int notificationAreaIconsWidth = ScaleTaskbarMetricForWindow(hWnd, GetCachedNotificationAreaIconsWidth(hWnd));
    int hiddenTrayStart = controlEnd + notificationAreaIconsWidth;
    int hiddenTrayEnd = hiddenTrayStart + ScaleTaskbarMetricForWindow(hWnd, kNotifyIconStackWidth);
    WPARAM result = 0;
    bool inControlCenterRange = false;

    if (WantsNotificationCenter() && xFromRight >= notificationStart && xFromRight < notificationEnd) {
        result = kProxyNotificationCenter;
    } else if (WantsTray() && xFromRight >= controlEnd && xFromRight < hiddenTrayEnd + hitSlop) {
        result = kNativeHiddenTray;
    } else if (WantsControlCenter() && xFromRight >= controlStart - hitSlop && xFromRight < controlEnd) {
        result = kNativeControlCenter;
        inControlCenterRange = true;
    } else if (WantsTray() && xFromRight >= hiddenTrayStart - hitSlop && xFromRight < hiddenTrayEnd + hitSlop) {
        result = kNativeHiddenTray;
    }

    if (g_settings.enableVerboseLogging && xFromRight >= notificationStart - hitSlop && xFromRight < hiddenTrayEnd + ScaleTaskbarMetricForWindow(hWnd, 32)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : secondary click monitor=%d x=%d y=%d "
            L"xFromRight=%d iconWidth=%d hit=%s",
            GetMonitorIndexForWindow(hWnd), x, y, xFromRight,
            notificationAreaIconsWidth,
            result == kProxyNotificationCenter
                ? L"notification center"
                : result == kProxyControlCenter
                    ? L"control center"
                    : result == kNativeHiddenTray
                        ? L"hidden tray native"
                        : result == kNativeControlCenter
                            ? L"control center native"
                            : inControlCenterRange
                                ? L"control center native"
                                : L"none"
        );
    }

    return result;
}

bool GetNativeFlyoutAnchorPoint(HWND hWnd, LPARAM lParam, WPARAM flyoutKind, POINT* anchorPoint) {
    RECT clientRect = {};

    if (!GetClientRect(hWnd, &clientRect)) {
        return false;
    }

    LONG clientX = GET_X_LPARAM(lParam);

    if (flyoutKind == kNativeHiddenTray) {
        int notificationAreaIconsWidth = ScaleTaskbarMetricForWindow(hWnd, GetCachedNotificationAreaIconsWidth(hWnd));
        int hiddenTrayStart = ScaleTaskbarMetricForWindow(hWnd, kShowDesktopWidth) + ScaleTaskbarMetricForWindow(hWnd, kNotificationCenterWidth) + ScaleTaskbarMetricForWindow(hWnd, kControlCenterWidth) + notificationAreaIconsWidth;
        int hiddenTrayWidth = ScaleTaskbarMetricForWindow(hWnd, kNotifyIconStackWidth);
        clientX = clientRect.right - (hiddenTrayStart + hiddenTrayWidth / 2);
        clientX = ClampLong(clientX, clientRect.left, clientRect.right - 1);
    }

    POINT point = {
        clientX,
        clientRect.top + (clientRect.bottom - clientRect.top) / 2,
    };

    if (!ClientToScreen(hWnd, &point)) {
        return false;
    }

    *anchorPoint = point;

    if (g_settings.enableVerboseLogging && flyoutKind == kNativeHiddenTray) {
        Wh_Log_safe(
            L"taskbar-multi-tray : anchored hidden tray flyout at "
            L"(%ld,%ld) for monitor %d",
            point.x, point.y, GetMonitorIndexForWindow(hWnd)
        );
    }

    return true;
}

bool SendWinHotkey(WORD vk) {
    INPUT inputs[4] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vk;
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = vk;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_LWIN;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT sent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    if (sent != ARRAYSIZE(inputs)) {
        Wh_Log_safe(L"taskbar-multi-tray : SendInput failed sent=%u error=%lu", sent, GetLastError());

        return false;
    }

    return true;
}

bool WaitForLeftButtonRelease(DWORD timeoutMs) {
    DWORD started = GetTickCount();

    while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        if (GetTickCount() - started >= timeoutMs) {
            return false;
        }

        Sleep(15);
    }

    return true;
}

bool SendMouseClickAtCursor() {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    UINT sent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    if (sent != ARRAYSIZE(inputs)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : SendInput mouse click failed sent=%u "
            L"error=%lu",
            sent, GetLastError()
        );

        return false;
    }

    return true;
}

bool ShouldRestoreAfterProxyOpen() {
    return g_settings.monitorMode == MonitorMode::All || g_settings.selectedMonitors.size() != 1;
}

void ClearProxyFlyoutMonitorState() {
    // Make the shared active context inactive before clearing the remaining metadata. ShellHost can read this block concurrently.
    g_sharedProxyState.proxyFlyoutMonitor = nullptr;
    g_sharedProxyState.proxyFlyoutUntilTick = 0;
    MemoryBarrier();

    g_proxyFlyoutMonitor = nullptr;
    g_proxyFlyoutMonitorIndex = 0;
    g_proxyFlyoutUntilTick = 0;
    g_proxyFlyoutGeneration = 0;
    g_proxyFlyoutKind = 0;
    g_proxyFlyoutTaskbarWnd = nullptr;
    g_proxyFlyoutAnchorValid = false;
    g_proxyFlyoutAnchorPoint = {};
    g_proxyFlyoutTaskbarRectValid = false;
    g_proxyFlyoutTaskbarRect = {};
    g_proxyFlyoutTargetDpi = 0;
    g_sharedProxyState.proxyFlyoutMonitorIndex = 0;
    g_sharedProxyState.proxyFlyoutKind = 0;
    g_sharedProxyState.proxyFlyoutTaskbarWnd = 0;
    g_sharedProxyState.proxyFlyoutAnchorValid = 0;
    g_sharedProxyState.proxyFlyoutAnchorX = 0;
    g_sharedProxyState.proxyFlyoutAnchorY = 0;
    g_sharedProxyState.proxyFlyoutTaskbarRectValid = 0;
    g_sharedProxyState.proxyFlyoutTaskbarLeft = 0;
    g_sharedProxyState.proxyFlyoutTaskbarTop = 0;
    g_sharedProxyState.proxyFlyoutTaskbarRight = 0;
    g_sharedProxyState.proxyFlyoutTaskbarBottom = 0;
    g_sharedProxyState.proxyFlyoutTargetDpi = 0;
    MemoryBarrier();
}

void ClearStaleFlyoutContextBeforeTaskbarClick(HWND taskbarWnd) {
    HMONITOR activeMonitor = GetActiveProxyFlyoutMonitor();

    if (!activeMonitor) {
        return;
    }

    HMONITOR taskbarMonitor = GetActualMonitorFromWindow(taskbarWnd, MONITOR_DEFAULTTONEAREST);
    if (!taskbarMonitor || taskbarMonitor == activeMonitor) {
        return;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : clearing stale flyout monitor context "
            L"before taskbar click monitor=%d active=%d",
            GetMonitorIndex(taskbarMonitor), GetMonitorIndex(activeMonitor)
        );
    }

    ClearProxyFlyoutMonitorState();
}

bool IsRightClickOrContextMenuMessage(UINT uMsg, WPARAM wParam) {
    return uMsg == WM_RBUTTONDOWN
        || uMsg == WM_RBUTTONUP
        || uMsg == WM_NCRBUTTONDOWN
        || uMsg == WM_NCRBUTTONUP
        || uMsg == WM_CONTEXTMENU
        || (uMsg == WM_PARENTNOTIFY && (LOWORD(wParam) == WM_RBUTTONDOWN || LOWORD(wParam) == WM_RBUTTONUP));
}

void ClearFlyoutMonitorContextForContextMenu(UINT uMsg, WPARAM wParam, PCWSTR reason) {
    if (!IsRightClickOrContextMenuMessage(uMsg, wParam) || !GetActiveProxyFlyoutMonitor()) {
        return;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : clearing flyout monitor context "
            L"before right-click/context menu message from %s",
            reason
        );
    }

    ClearProxyFlyoutMonitorState();
}

void InspectRetrievedMessageForFlyoutCancel(const MSG* msg, PCWSTR source) {
    if (!msg) {
        return;
    }

    ClearFlyoutMonitorContextForContextMenu(msg -> message, msg -> wParam, source);
}

LRESULT WINAPI DispatchMessageW_Hook(const MSG* lpMsg) {
    if (!IsModUnloading()) {
        InspectRetrievedMessageForFlyoutCancel(lpMsg, L"DispatchMessageW");
    }

    return DispatchMessageW_Original(lpMsg);
}

void KillTrackedNativeFlyoutTimers() {
    for (const auto& state : g_nativeFlyoutTimerStates) {
        if (state.hWnd) {
            KillTimer(state.hWnd, kClearNativeFlyoutMonitorTimerId);
        }
    }

    g_nativeFlyoutTimerStates.clear();
}

void ClearProxyRuntimeState() {
    g_runtimePrimaryTrayMonitor = nullptr;
    g_runtimePrimaryTrayMonitorIndex = 0;
    g_sharedProxyState.runtimePrimaryTrayMonitor = nullptr;
    g_sharedProxyState.runtimePrimaryTrayMonitorIndex = 0;
    ClearProxyFlyoutMonitorState();
    KillTrackedNativeFlyoutTimers();
}

void ClearCachedXamlBindings() {
    g_primaryNotificationAreaIconsBinding = {};
    g_primaryNotifyIconStackBinding = {};
    g_primaryNotifyIconStackChildBinding = {};
    g_primaryNotifyIconStackListViewBinding = {};
    g_primaryControlCenterButtonBinding = {};
}

double GetCachedNotificationAreaIconsWidth(HWND hWnd) {
    for (const auto& metrics : g_taskbarTrayMetrics) {
        if (metrics.hWnd == hWnd) {
            return metrics.notificationAreaIconsWidth;
        }
    }

    return 0.0;
}

void SetCachedNotificationAreaIconsWidth(HWND hWnd, double width) {
    for (auto& metrics : g_taskbarTrayMetrics) {
        if (metrics.hWnd == hWnd) {
            metrics.notificationAreaIconsWidth = width;

            return;
        }
    }

    g_taskbarTrayMetrics.push_back({hWnd, width});
}

void RemoveCachedTaskbarTrayMetrics(HWND hWnd) {
    for (auto it = g_taskbarTrayMetrics.begin(); it != g_taskbarTrayMetrics.end(); ++it) {
        if (it -> hWnd == hWnd) {
            g_taskbarTrayMetrics.erase(it);

            return;
        }
    }
}

void RememberNativeFlyoutTimerGeneration(HWND hWnd, LONG generation) {
    if (!hWnd || !generation) {
        return;
    }

    for (auto& state : g_nativeFlyoutTimerStates) {
        if (state.hWnd == hWnd) {
            state.generation = generation;

            return;
        }
    }

    g_nativeFlyoutTimerStates.push_back({hWnd, generation});
}

LONG GetNativeFlyoutTimerGeneration(HWND hWnd) {
    for (const auto& state : g_nativeFlyoutTimerStates) {
        if (state.hWnd == hWnd) {
            return state.generation;
        }
    }

    return 0;
}

void RemoveNativeFlyoutTimerGeneration(HWND hWnd) {
    for (auto it = g_nativeFlyoutTimerStates.begin(); it != g_nativeFlyoutTimerStates.end(); ++it) {
        if (it -> hWnd == hWnd) {
            g_nativeFlyoutTimerStates.erase(it);

            return;
        }
    }
}

void CancelDeferredApplySettings() {
    InterlockedIncrement(&g_deferredApplyGeneration);

    if (g_deferredApplyTimerWnd) {
        KillTimer(g_deferredApplyTimerWnd, kDeferredApplySettingsTimerId);
    }

    g_deferredApplyTimerWnd = nullptr;
    g_deferredApplyTimerGeneration = 0;
    g_deferredApplyRetryIndex = 0;
}

void CancelTaskbarTimers(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    KillTimer(hWnd, kRestoreProxyTrayTimerId);
    KillTimer(hWnd, kClearNativeFlyoutMonitorTimerId);
    RemoveNativeFlyoutTimerGeneration(hWnd);

    if (g_deferredApplyTimerWnd == hWnd) {
        KillTimer(hWnd, kDeferredApplySettingsTimerId);
        g_deferredApplyTimerWnd = nullptr;
        g_deferredApplyTimerGeneration = 0;
        g_deferredApplyRetryIndex = 0;
    }
}

bool ScheduleNextDeferredApplySettingsTimer(HWND taskbarWnd) {
    if (!taskbarWnd || IsModUnloading() || g_deferredApplyRetryIndex >= ARRAYSIZE(kDeferredApplyRetryDelaysMs)) {
        g_deferredApplyTimerWnd = nullptr;
        g_deferredApplyTimerGeneration = 0;
        g_deferredApplyRetryIndex = 0;

        return false;
    }

    DWORD delayMs = kDeferredApplyRetryDelaysMs[g_deferredApplyRetryIndex];

    if (!SetTimer(taskbarWnd, kDeferredApplySettingsTimerId, delayMs, nullptr)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : failed to schedule deferred apply "
            L"timer error=%lu",
            GetLastError()
        );
        g_deferredApplyTimerWnd = nullptr;
        g_deferredApplyTimerGeneration = 0;
        g_deferredApplyRetryIndex = 0;

        return false;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : scheduled deferred apply retry %u "
            L"after %lu ms generation=%ld hwnd=0x%p",
            static_cast<unsigned>(g_deferredApplyRetryIndex + 1),
            delayMs, g_deferredApplyTimerGeneration, taskbarWnd
        );
    }

    return true;
}

void HandleDeferredApplySettingsTimer(HWND taskbarWnd) {
    KillTimer(taskbarWnd, kDeferredApplySettingsTimerId);

    if (IsModUnloading() || taskbarWnd != g_deferredApplyTimerWnd || !g_deferredApplyTimerGeneration || g_deferredApplyTimerGeneration != g_deferredApplyGeneration) {
        return;
    }

    if (g_deferredApplyRetryIndex >= ARRAYSIZE(kDeferredApplyRetryDelaysMs)) {
        g_deferredApplyTimerWnd = nullptr;
        g_deferredApplyTimerGeneration = 0;
        g_deferredApplyRetryIndex = 0;

        return;
    }

    DWORD delayMs = kDeferredApplyRetryDelaysMs[g_deferredApplyRetryIndex];

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : deferred apply retry from taskbar "
            L"timer %u after %lu ms generation=%ld hwnd=0x%p",
            static_cast<unsigned>(g_deferredApplyRetryIndex + 1),
            delayMs, g_deferredApplyTimerGeneration, taskbarWnd
        );
    }

    NotifyTaskbarDisplayChange(taskbarWnd);
    ApplySettingsFromTaskbarThread(nullptr);

    g_deferredApplyRetryIndex++;
    ScheduleNextDeferredApplySettingsTimer(taskbarWnd);
}

void RestoreProxyTrayTarget(HWND taskbarWnd) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : restoring proxy tray target");
    }

    ClearProxyRuntimeState();
    NotifyTaskbarDisplayChange(FindCurrentProcessTaskbarWnd());
    ApplySettingsFromTaskbarThread(nullptr);
}

bool SetFlyoutMonitorContext(HWND taskbarWnd, DWORD durationMs, PCWSTR reason, WPARAM flyoutKind = 0, const POINT* anchorPoint = nullptr) {
    HMONITOR monitor = GetActualMonitorFromWindow(taskbarWnd, MONITOR_DEFAULTTONEAREST);

    if (!monitor) {
        return false;
    }

    int monitorIndex = GetMonitorIndex(monitor);
    DWORD untilTick = GetTickCount() + durationMs;
    UINT targetDpi = GetMonitorDpiOrDefault(monitor);

    if (!targetDpi) {
        targetDpi = GetWindowDpiOrDefault(taskbarWnd);
    }

    RECT taskbarRect = {};
    bool taskbarRectValid = GetWindowRect(taskbarWnd, &taskbarRect) != FALSE;

    // Publish the cross-process ShellHost context in two phases. During rapid clicks across different-DPI monitors, ShellHost can read the shared block while Explorer is replacing the old monitor. Keep the active monitor null until DPI/anchor/taskbar metadata is refreshed.
    g_sharedProxyState.proxyFlyoutMonitor = nullptr;
    g_sharedProxyState.proxyFlyoutUntilTick = 0;
    MemoryBarrier();

    g_proxyFlyoutMonitor = monitor;
    g_proxyFlyoutMonitorIndex = monitorIndex;
    g_proxyFlyoutUntilTick = untilTick;
    g_proxyFlyoutGeneration = InterlockedIncrement(reinterpret_cast<volatile LONG*>(&g_sharedProxyState.proxyFlyoutGeneration));
    g_proxyFlyoutKind = static_cast<int>(flyoutKind);
    g_proxyFlyoutTaskbarWnd = taskbarWnd;
    g_proxyFlyoutTargetDpi = targetDpi;

    g_sharedProxyState.proxyFlyoutMonitorIndex = monitorIndex;
    g_sharedProxyState.proxyFlyoutKind = g_proxyFlyoutKind;
    g_sharedProxyState.proxyFlyoutTaskbarWnd = reinterpret_cast<LONG_PTR>(taskbarWnd);
    g_sharedProxyState.proxyFlyoutTargetDpi = targetDpi;

    if (taskbarRectValid) {
        g_proxyFlyoutTaskbarRectValid = true;
        g_proxyFlyoutTaskbarRect = taskbarRect;
        g_sharedProxyState.proxyFlyoutTaskbarRectValid = 1;
        g_sharedProxyState.proxyFlyoutTaskbarLeft = taskbarRect.left;
        g_sharedProxyState.proxyFlyoutTaskbarTop = taskbarRect.top;
        g_sharedProxyState.proxyFlyoutTaskbarRight = taskbarRect.right;
        g_sharedProxyState.proxyFlyoutTaskbarBottom = taskbarRect.bottom;
    } else {
        g_proxyFlyoutTaskbarRectValid = false;
        g_proxyFlyoutTaskbarRect = {};
        g_sharedProxyState.proxyFlyoutTaskbarRectValid = 0;
        g_sharedProxyState.proxyFlyoutTaskbarLeft = 0;
        g_sharedProxyState.proxyFlyoutTaskbarTop = 0;
        g_sharedProxyState.proxyFlyoutTaskbarRight = 0;
        g_sharedProxyState.proxyFlyoutTaskbarBottom = 0;
    }

    if (anchorPoint) {
        g_proxyFlyoutAnchorValid = true;
        g_proxyFlyoutAnchorPoint = *anchorPoint;
        g_sharedProxyState.proxyFlyoutAnchorValid = 1;
        g_sharedProxyState.proxyFlyoutAnchorX = anchorPoint -> x;
        g_sharedProxyState.proxyFlyoutAnchorY = anchorPoint -> y;
    } else {
        g_proxyFlyoutAnchorValid = false;
        g_proxyFlyoutAnchorPoint = {};
        g_sharedProxyState.proxyFlyoutAnchorValid = 0;
        g_sharedProxyState.proxyFlyoutAnchorX = 0;
        g_sharedProxyState.proxyFlyoutAnchorY = 0;
    }

    MemoryBarrier();
    g_sharedProxyState.proxyFlyoutUntilTick = untilTick;
    g_sharedProxyState.proxyFlyoutMonitor = monitor;

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : %s flyout monitor context monitor=%d "
            L"until tick=%lu generation=%ld kind=%d anchor=%d (%ld,%ld) "
            L"taskbar=(%ld,%ld,%ld,%ld) dpi=%u clicked=0x%p",
            reason, monitorIndex, g_proxyFlyoutUntilTick,
            g_proxyFlyoutGeneration, static_cast<int>(flyoutKind),
            anchorPoint != nullptr,
            anchorPoint ? anchorPoint -> x : 0,
            anchorPoint ? anchorPoint -> y : 0,
            g_proxyFlyoutTaskbarRect.left, g_proxyFlyoutTaskbarRect.top,
            g_proxyFlyoutTaskbarRect.right,
            g_proxyFlyoutTaskbarRect.bottom, g_proxyFlyoutTargetDpi,
            taskbarWnd
        );
    }

    return true;
}

void BeginNativeFlyoutMonitorContext(HWND taskbarWnd, PCWSTR reason, WPARAM flyoutKind, const POINT* anchorPoint) {
    if (SetFlyoutMonitorContext(taskbarWnd, kNativeFlyoutMonitorContextMs, reason, flyoutKind, anchorPoint)) {
        KillTimer(taskbarWnd, kClearNativeFlyoutMonitorTimerId);
        RememberNativeFlyoutTimerGeneration(taskbarWnd, g_proxyFlyoutGeneration);
        SetTimer(taskbarWnd, kClearNativeFlyoutMonitorTimerId, kNativeFlyoutMonitorContextMs, nullptr);
    }
}

void OpenProxyFlyout(HWND taskbarWnd, WPARAM flyout) {
    HMONITOR monitor = GetActualMonitorFromWindow(taskbarWnd, MONITOR_DEFAULTTONEAREST);

    if (!monitor) {
        return;
    }

    g_runtimePrimaryTrayMonitor = monitor;
    g_runtimePrimaryTrayMonitorIndex = GetMonitorIndex(monitor);
    g_sharedProxyState.runtimePrimaryTrayMonitor = monitor;
    g_sharedProxyState.runtimePrimaryTrayMonitorIndex = g_runtimePrimaryTrayMonitorIndex;
    SetFlyoutMonitorContext(taskbarWnd, 5000, L"proxy", flyout);

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : proxy opening %s from monitor %d until "
            L"tick=%lu",
            flyout == kProxyNotificationCenter ? L"notification center" : L"control center",
            g_runtimePrimaryTrayMonitorIndex, g_proxyFlyoutUntilTick
        );
    }

    HWND primaryTaskbarWnd = FindCurrentProcessTaskbarWnd();
    NotifyTaskbarDisplayChange(primaryTaskbarWnd);

    Sleep(120);

    if (flyout == kProxyControlCenter) {
        WaitForLeftButtonRelease(800);

        if (g_settings.enableVerboseLogging) {
            POINT cursor = {};
            GetCursorPos(&cursor);
            Wh_Log_safe(L"taskbar-multi-tray : clicking moved control center at cursor (%ld,%ld)", cursor.x, cursor.y);
        }

        SendMouseClickAtCursor();
    } else {
        SendWinHotkey(L'N');
    }

    if (ShouldRestoreAfterProxyOpen()) {
        HWND timerWnd = FindCurrentProcessTaskbarWnd();
        SetTimer(timerWnd ? timerWnd : taskbarWnd, kRestoreProxyTrayTimerId, 2500, nullptr);
    }
}

bool PostProxyOpenFlyout(HWND hWnd, WPARAM flyout) {
    DWORD now = GetTickCount();

    if (g_lastProxyPostWnd == hWnd && g_lastProxyPostFlyout == flyout &&
        now - g_lastProxyPostTick < 350) {

        return true;
    }

    g_lastProxyPostWnd = hWnd;
    g_lastProxyPostFlyout = flyout;
    g_lastProxyPostTick = now;

    return PostMessage(hWnd, ProxyOpenMessage(), flyout, 0) != FALSE;
}

LRESULT CALLBACK TaskbarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (IsModUnloading()) {
        if (uMsg == ProxyOpenMessage()) {
            return 0;
        }

        if (
            uMsg == WM_TIMER &&
            (wParam == kRestoreProxyTrayTimerId || wParam == kClearNativeFlyoutMonitorTimerId || wParam == kDeferredApplySettingsTimerId)
        ) {
            CancelTaskbarTimers(hWnd);

            return 0;
        }

        if (uMsg == WM_NCDESTROY) {
            RemoveCachedTaskbarTrayMetrics(hWnd);
            CancelTaskbarTimers(hWnd);
            RemoveWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == ProxyOpenMessage()) {
        OpenProxyFlyout(hWnd, wParam);

        return 0;
    }

    if (uMsg == WM_TIMER && wParam == kRestoreProxyTrayTimerId) {
        KillTimer(hWnd, kRestoreProxyTrayTimerId);
        RestoreProxyTrayTarget(hWnd);

        return 0;
    }

    if (uMsg == WM_TIMER && wParam == kDeferredApplySettingsTimerId) {
        HandleDeferredApplySettingsTimer(hWnd);

        return 0;
    }

    if (uMsg == WM_TIMER && wParam == kClearNativeFlyoutMonitorTimerId) {
        KillTimer(hWnd, kClearNativeFlyoutMonitorTimerId);
        LONG timerGeneration = GetNativeFlyoutTimerGeneration(hWnd);
        RemoveNativeFlyoutTimerGeneration(hWnd);

        if (!GetActiveProxyFlyoutMonitor()) {
            return 0;
        }

        LONG activeGeneration = GetActiveProxyFlyoutGeneration();

        if (timerGeneration && activeGeneration && timerGeneration != activeGeneration) {
            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : ignoring stale native flyout "
                    L"context timer generation=%ld active=%ld",
                    timerGeneration, activeGeneration
                );
            }

            return 0;
        }

        if (ActiveFlyoutContextStillHasTime()) {
            if (g_settings.enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : ignoring native flyout context "
                    L"timer generation=%ld until tick=%lu",
                    activeGeneration, GetActiveProxyFlyoutUntilTick()
                );
            }

            return 0;
        }

        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : clearing native flyout monitor "
                L"context generation=%ld",
                activeGeneration
            );
        }
    
        ClearProxyFlyoutMonitorState();

        return 0;
    }

    if (IsRightClickOrContextMenuMessage(uMsg, wParam)) {
        ClearFlyoutMonitorContextForContextMenu(uMsg, wParam, L"taskbar");
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    bool leftButtonDown = uMsg == WM_LBUTTONDOWN || (uMsg == WM_PARENTNOTIFY && LOWORD(wParam) == WM_LBUTTONDOWN);
    bool leftButtonUp = uMsg == WM_LBUTTONUP || (uMsg == WM_PARENTNOTIFY && LOWORD(wParam) == WM_LBUTTONUP);

    if (leftButtonDown || leftButtonUp) {
        WPARAM proxyFlyout = HitTestProxyFlyout(hWnd, lParam);

        if (proxyFlyout) {
            if (proxyFlyout == kNativeControlCenter) {
                BeginNativeFlyoutMonitorContext(hWnd, L"native control center", proxyFlyout, nullptr);

                return DefSubclassProc(hWnd, uMsg, wParam, lParam);
            }

            if (proxyFlyout == kNativeHiddenTray) {
                POINT anchorPoint = {};
                POINT* anchorPointPtr = GetNativeFlyoutAnchorPoint(hWnd, lParam, proxyFlyout, &anchorPoint)
                    ? &anchorPoint
                    : nullptr;
                BeginNativeFlyoutMonitorContext(hWnd, L"native hidden tray", proxyFlyout, anchorPointPtr);

                return DefSubclassProc(hWnd, uMsg, wParam, lParam);
            }

            if (leftButtonDown) {
                if (!PostProxyOpenFlyout(hWnd, proxyFlyout)) {
                    Wh_Log_safe(L"taskbar-multi-tray : PostMessage proxy open failed error=%lu", GetLastError());
                }

                return 0;
            }
        } else {
            ClearStaleFlyoutContextBeforeTaskbarClick(hWnd);
        }
    } else if (uMsg == WM_NCDESTROY) {
        RemoveCachedTaskbarTrayMetrics(hWnd);
        CancelTaskbarTimers(hWnd);
        RemoveWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId);
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void InstallTaskbarSubclass(HWND hWnd) {
    if (!SetWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId, 0)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : SetWindowSubclass failed for hwnd=0x%p "
            L"error=%lu",
            hWnd, GetLastError()
        );
    }
}

void RemoveTaskbarSubclass(HWND hWnd) {
    CancelTaskbarTimers(hWnd);
    RemoveWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId);
}

XamlRoot XamlRootFromTaskbarHostSharedPtr(void* taskbarHostSharedPtr[2]) {
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1]) {
        return nullptr;
    }

    size_t taskbarElementIUnknownOffset = 0x48;

#if defined(_M_X64)
{
    const BYTE* b = static_cast<const BYTE*>(TaskbarHost_FrameHeight_Original);

    if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
        b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F) {
        taskbarElementIUnknownOffset = b[7];
    } else {
        Wh_Log_safe(L"taskbar-multi-tray : unsupported TaskbarHost::FrameHeight");
    }
}
#else
#error "Unsupported architecture"
#endif

    auto* taskbarElementIUnknown = *reinterpret_cast<IUnknown**>(static_cast<BYTE*>(taskbarHostSharedPtr[0]) + taskbarElementIUnknownOffset);

    FrameworkElement taskbarElement = nullptr;
    taskbarElementIUnknown -> QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(taskbarElement));

    auto result = taskbarElement ? taskbarElement.XamlRoot() : nullptr;

    std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);

    return result;
}

XamlRoot GetTaskbarXamlRoot(HWND taskbarWnd) {
    HWND taskSwWnd = reinterpret_cast<HWND>(GetProp(taskbarWnd, L"TaskbandHWND"));

    if (!taskSwWnd) {
        return nullptr;
    }

    void* taskBand = reinterpret_cast<void*>(GetWindowLongPtr(taskSwWnd, 0));
    void* taskBandForTaskListWndSite = taskBand;

    for (int i = 0; *reinterpret_cast<void**>(taskBandForTaskListWndSite) != CTaskBand_ITaskListWndSite_vftable; i++) {
        if (i == 20) {
            return nullptr;
        }

        taskBandForTaskListWndSite = reinterpret_cast<void**>(taskBandForTaskListWndSite) + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForTaskListWndSite, taskbarHostSharedPtr);

    return XamlRootFromTaskbarHostSharedPtr(taskbarHostSharedPtr);
}

bool GetSecondaryTaskbarHostSharedPtr(HWND secondaryTaskbarWnd, void* taskbarHostSharedPtr[2]) {
    HWND taskSwWnd = FindWindowEx(secondaryTaskbarWnd, nullptr, L"WorkerW", nullptr);

    if (!taskSwWnd) {
        return false;
    }

    void* taskBand = reinterpret_cast<void*>(GetWindowLongPtr(taskSwWnd, 0));
    void* taskBandForTaskListWndSite = taskBand;

    for (int i = 0; *reinterpret_cast<void**>(taskBandForTaskListWndSite) != CSecondaryTaskBand_ITaskListWndSite_vftable; i++) {
        if (i == 20) {
            return false;
        }

        taskBandForTaskListWndSite = reinterpret_cast<void**>(taskBandForTaskListWndSite) + 1;
    }

    CSecondaryTaskBand_GetTaskbarHost_Original(taskBandForTaskListWndSite, taskbarHostSharedPtr);

    return taskbarHostSharedPtr[0] != nullptr;
}

XamlRoot GetSecondaryTaskbarXamlRoot(HWND secondaryTaskbarWnd) {
    void* taskbarHostSharedPtr[2]{};

    if (!GetSecondaryTaskbarHostSharedPtr(secondaryTaskbarWnd, taskbarHostSharedPtr)) {
        return nullptr;
    }

    return XamlRootFromTaskbarHostSharedPtr(taskbarHostSharedPtr);
}

using RunFromWindowThreadProc_t = void(WINAPI*)(void* parameter);

bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc, void* procParam) {
    static const UINT runFromWindowThreadRegisteredMsg = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);

    struct RunFromWindowThreadParam {
        RunFromWindowThreadProc_t proc;
        void* procParam;
    };

    DWORD threadId = GetWindowThreadProcessId(hWnd, nullptr);

    if (!threadId) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to get window thread for hwnd=0x%p", hWnd);

        return false;
    }

    if (threadId == GetCurrentThreadId()) {
        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : already on taskbar thread %lu", threadId);
        }

        proc(procParam);

        return true;
    }

    HHOOK hook = SetWindowsHookEx(
        WH_CALLWNDPROC,
        [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (nCode == HC_ACTION) {
                const CWPSTRUCT* cwp = reinterpret_cast<const CWPSTRUCT*>(lParam);

                if (cwp -> message == runFromWindowThreadRegisteredMsg) {
                    auto* param = reinterpret_cast<RunFromWindowThreadParam*>(cwp -> lParam);
                    param -> proc(param -> procParam);
                }
            }

            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        },
        nullptr, threadId
    );

    if (!hook) {
        Wh_Log_safe(
            L"taskbar-multi-tray : SetWindowsHookEx failed for thread %lu, "
            L"error=%lu",
            threadId, GetLastError()
        );

        return false;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : dispatching to taskbar thread %lu", threadId);
    }

    RunFromWindowThreadParam param{proc, procParam};
    SendMessage(hWnd, runFromWindowThreadRegisteredMsg, 0, reinterpret_cast<LPARAM>(&param));

    UnhookWindowsHookEx(hook);

    return true;
}

void ApplySettingsFromTaskbarThread(void*) {
    if (IsModUnloading()) {
        return;
    }

    ActiveFlyoutRedirectionSuppressor suppressActiveFlyoutRedirection;

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : applying from thread %lu", GetCurrentThreadId());
    }

    std::vector<HWND> taskbarWindows;
    EnumThreadWindows(
        GetCurrentThreadId(),
        [](HWND hWnd, LPARAM lParam) -> BOOL {
            wchar_t className[32] = {};

            if (!GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
                return TRUE;
            }

            if (_wcsicmp(className, L"Shell_TrayWnd") == 0 ||
                _wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0) {
                auto* windows = reinterpret_cast<std::vector<HWND>*>(lParam);
                windows -> push_back(hWnd);
            }

            return TRUE;
        },
        reinterpret_cast<LPARAM>(&taskbarWindows)
    );

    for (size_t i = 0; i < taskbarWindows.size(); i++) {
        if (IsRealPrimaryTrayTargetWindow(taskbarWindows[i])) {
            HWND preferred = taskbarWindows[i];
            taskbarWindows.erase(taskbarWindows.begin() + i);
            taskbarWindows.insert(taskbarWindows.begin(), preferred);

            break;
        }
    }

    for (HWND hWnd : taskbarWindows) {
        wchar_t className[32] = {};

        if (!GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
            continue;
        }

        XamlRoot xamlRoot = nullptr;

        if (_wcsicmp(className, L"Shell_TrayWnd") == 0) {
            xamlRoot = GetTaskbarXamlRoot(hWnd);
        } else if (_wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0) {
            xamlRoot = GetSecondaryTaskbarXamlRoot(hWnd);
        } else {
            continue;
        }

        InstallTaskbarSubclass(hWnd);

        if (!xamlRoot) {
            int monitorIndex = GetMonitorIndexForWindow(hWnd);
            Wh_Log_safe(L"taskbar-multi-tray : XamlRoot not found for monitor %d", monitorIndex);

            continue;
        }

        if (!ApplyStyle(xamlRoot, hWnd)) {
            int monitorIndex = GetMonitorIndexForWindow(hWnd);
            Wh_Log_safe(L"taskbar-multi-tray : apply failed for monitor %d", monitorIndex);
        }
    }
}

void RemoveTaskbarSubclassesFromTaskbarThread(void*) {
    EnumThreadWindows(
        GetCurrentThreadId(),
        [](HWND hWnd, LPARAM) -> BOOL {
            wchar_t className[32] = {};

            if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) && (_wcsicmp(className, L"Shell_TrayWnd") == 0 || _wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0)) {
                RemoveTaskbarSubclass(hWnd);

                if (g_settings.enableVerboseLogging) {
                    Wh_Log_safe(L"taskbar-multi-tray : removed subclass/timers from taskbar hwnd=0x%p monitor=%d", hWnd, GetMonitorIndexForWindow(hWnd));
                }
            }

            return TRUE;
        },
        0
    );
}

void RestoreNativeTaskbarsFromTaskbarThread(void*) {
    ActiveFlyoutRedirectionSuppressor suppressActiveFlyoutRedirection;

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : restoring native taskbar XAML state from " L"thread %lu", GetCurrentThreadId());
    }

    EnumThreadWindows(
        GetCurrentThreadId(),
        [](HWND hWnd, LPARAM) -> BOOL {
            wchar_t className[32] = {};

            if (!GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
                return TRUE;
            }

            XamlRoot xamlRoot = nullptr;
            bool primaryTaskbar = _wcsicmp(className, L"Shell_TrayWnd") == 0;

            if (primaryTaskbar) {
                xamlRoot = GetTaskbarXamlRoot(hWnd);
            } else if (_wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0) {
                xamlRoot = GetSecondaryTaskbarXamlRoot(hWnd);
            } else {
                return TRUE;
            }

            if (!xamlRoot) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : XamlRoot not found while "
                    L"restoring native state for monitor %d",
                    GetMonitorIndexForWindow(hWnd)
                );

                return TRUE;
            }

            if (primaryTaskbar) {
                ApplyNativePrimaryStyle(xamlRoot, hWnd);
            } else {
                ApplyNonTargetStyle(xamlRoot, hWnd);
            }

            return TRUE;
        },
        0
    );
}

void ApplySettings() {
    HWND taskbarWnd = FindCurrentProcessTaskbarWnd();

    if (!taskbarWnd) {
        Wh_Log_safe(L"taskbar-multi-tray : no taskbar found");

        return;
    }

    NotifyTaskbarDisplayChange(taskbarWnd);
    LogTaskbarWindow(taskbarWnd, L"ApplySettings primary");

    if (!RunFromWindowThread(taskbarWnd, ApplySettingsFromTaskbarThread, nullptr)) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to run on taskbar thread");
    }
}

void QueueDeferredApplySettings() {
    if (!IsExplorerTarget() || IsModUnloading()) {
        return;
    }

    HWND taskbarWnd = FindCurrentProcessTaskbarWnd();

    if (!taskbarWnd) {
        Wh_Log_safe(L"taskbar-multi-tray : no taskbar found for deferred apply");

        return;
    }

    CancelDeferredApplySettings();
    g_deferredApplyTimerWnd = taskbarWnd;
    g_deferredApplyTimerGeneration = InterlockedIncrement(&g_deferredApplyGeneration);
    g_deferredApplyRetryIndex = 0;
    ScheduleNextDeferredApplySettingsTimer(taskbarWnd);
}

using TrayUI_StartTaskbar_t = void(WINAPI*)(void* pThis);
TrayUI_StartTaskbar_t TrayUI_StartTaskbar_Original;
void WINAPI TrayUI_StartTaskbar_Hook(void* pThis) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : TrayUI::StartTaskbar hook");
    }

    TrayUI_StartTaskbar_Original(pThis);

    if (IsModUnloading()) {
        return;
    }

    ApplySettingsFromTaskbarThread(nullptr);
    QueueDeferredApplySettings();
}

using TaskbarHost_Ctor_t = void*(WINAPI*)(void* pThis, void* trayComponentHost, void* taskbarModel, void* taskbarControllerExtension, HMONITOR monitor, HWND hWnd, bool isPrimary, int colorTheme, void* secondaryTray);
TaskbarHost_Ctor_t TaskbarHost_Ctor_Original;
void* WINAPI TaskbarHost_Ctor_Hook(void* pThis, void* trayComponentHost, void* taskbarModel, void* taskbarControllerExtension, HMONITOR monitor, HWND hWnd, bool isPrimary, int colorTheme, void* secondaryTray) {
    if (IsModUnloading()) {
        return TaskbarHost_Ctor_Original(pThis, trayComponentHost, taskbarModel, taskbarControllerExtension, monitor, hWnd, isPrimary, colorTheme, secondaryTray);
    }

    bool forcePrimaryLike =
        secondaryTray && g_inSecondaryTrayInit && monitor &&
        ShouldApplyToMonitor(monitor) && (WantsTray() || WantsControlCenter());

    if (forcePrimaryLike && !isPrimary) {
        isPrimary = true;
    }

    g_secondaryTaskbarHostPrimaryLike = g_secondaryTaskbarHostPrimaryLike || forcePrimaryLike || isPrimary;

    if (g_settings.enableVerboseLogging && (g_inSecondaryTrayInit || secondaryTray || forcePrimaryLike)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : TaskbarHost ctor this=0x%p monitor=%d "
            L"hwnd=0x%p secondaryTray=0x%p primaryLike=%d forced=%d",
            pThis, GetMonitorIndex(monitor), hWnd, secondaryTray,
            isPrimary, forcePrimaryLike
        );
    }

    void* ret = TaskbarHost_Ctor_Original(pThis, trayComponentHost, taskbarModel, taskbarControllerExtension, monitor, hWnd, isPrimary, colorTheme, secondaryTray);

    return ret ? ret : pThis;
}

void WINAPI TaskbarModel_SetNotificationAreaIconManager2_Hook(
    void* pThis,
    void* notificationAreaIconManagerSharedPtr) {
    void* manager = GetSharedPtrObject(notificationAreaIconManagerSharedPtr);
    void* controlBlock = GetSharedPtrControlBlock(notificationAreaIconManagerSharedPtr);

    if (g_inSecondaryTrayInit) {
        g_lastSecondaryNotificationAreaIconManager = manager;
    } else {
        g_lastPrimaryNotificationAreaIconManager = manager;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : TaskbarModel::SetNotificationAreaIcon"
            L"Manager2 model=0x%p manager=0x%p control=0x%p "
            L"secondaryInit=%d monitor=%d primaryManager=0x%p "
            L"secondaryManager=0x%p",
            pThis, manager, controlBlock, g_inSecondaryTrayInit,
            g_secondaryInitMonitorIndex,
            g_lastPrimaryNotificationAreaIconManager,
            g_lastSecondaryNotificationAreaIconManager
        );
    }

    TaskbarModel_SetNotificationAreaIconManager2_Original(pThis, notificationAreaIconManagerSharedPtr);
}

HRESULT WINAPI TaskbarModel_GetNotificationAreaPromotedIcons_Hook(void* pThis, void** result) {
    HRESULT hr = TaskbarModel_GetNotificationAreaPromotedIcons_Original(pThis, result);
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : TaskbarModel::get_NotificationArea"
            L"PromotedIcons model=0x%p hr=0x%08X result=0x%p "
            L"secondaryInit=%d monitor=%d",
            pThis, hr, result ? *result : nullptr, g_inSecondaryTrayInit,
            g_secondaryInitMonitorIndex
        );
    }

    return hr;
}

HRESULT WINAPI TaskbarModel_GetNotificationAreaOverflowIcons_Hook(void* pThis, void** result) {
    HRESULT hr = TaskbarModel_GetNotificationAreaOverflowIcons_Original(pThis, result);

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : TaskbarModel::get_NotificationArea"
            L"OverflowIcons model=0x%p hr=0x%08X result=0x%p "
            L"secondaryInit=%d monitor=%d",
            pThis, hr, result ? *result : nullptr, g_inSecondaryTrayInit,
            g_secondaryInitMonitorIndex
        );
    }

    return hr;
}

void WINAPI NotificationAreaIconManager2_AddIconToVisibleCollection_Hook(void* pThis, void* icon) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : NotificationAreaIconManager2::"
            L"AddIconToVisibleCollection manager=0x%p icon=0x%p "
            L"matchesPrimary=%d matchesSecondary=%d",
            pThis, icon, pThis == g_lastPrimaryNotificationAreaIconManager,
            pThis == g_lastSecondaryNotificationAreaIconManager
        );
    }

    NotificationAreaIconManager2_AddIconToVisibleCollection_Original(pThis, icon);
}

void WINAPI NotificationAreaIconManager2_RemoveIconFromVisibleCollection_Hook(void* pThis, void* icon) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : NotificationAreaIconManager2::"
            L"RemoveIconFromVisibleCollection manager=0x%p icon=0x%p "
            L"matchesPrimary=%d matchesSecondary=%d",
            pThis, icon, pThis == g_lastPrimaryNotificationAreaIconManager,
            pThis == g_lastSecondaryNotificationAreaIconManager
        );
    }

    NotificationAreaIconManager2_RemoveIconFromVisibleCollection_Original(pThis, icon);
}

bool WINAPI NotificationAreaIconManager2_ShellNotifyIcon_Hook(void* pThis, void* trayNotifyData) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : NotificationAreaIconManager2::"
            L"ShellNotifyIcon manager=0x%p data=0x%p matchesPrimary=%d "
            L"matchesSecondary=%d",
            pThis, trayNotifyData,
            pThis == g_lastPrimaryNotificationAreaIconManager,
            pThis == g_lastSecondaryNotificationAreaIconManager
        );
    }

    return NotificationAreaIconManager2_ShellNotifyIcon_Original(pThis, trayNotifyData);
}

using SystemTrayHost_TryLoadNotificationAreaFrameFromXamlExtension_t = void(WINAPI*)(void* pThis);
SystemTrayHost_TryLoadNotificationAreaFrameFromXamlExtension_t SystemTrayHost_TryLoadNotificationAreaFrameFromXamlExtension_Original;
void WINAPI SystemTrayHost_TryLoadNotificationAreaFrameFromXamlExtension_Hook(void* pThis) {
    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : SystemTrayHost::TryLoadNotificationArea"
            L"FrameFromXamlExtension this=0x%p secondaryInit=%d "
            L"monitor=%d primaryLikeHost=%d",
            pThis, g_inSecondaryTrayInit, g_secondaryInitMonitorIndex,
            g_secondaryTaskbarHostPrimaryLike
        );
    }

    SystemTrayHost_TryLoadNotificationAreaFrameFromXamlExtension_Original(pThis);
}

void WINAPI TaskbarHost_ActivateOverflowFlyout_Hook(void* pThis) {
    if (IsModUnloading()) {
        TaskbarHost_ActivateOverflowFlyout_Original(pThis);

        return;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : TaskbarHost::ActivateOverflowFlyout hook this=0x%p activeFlyoutMonitor=%d", pThis, g_proxyFlyoutMonitorIndex);
    }

    TaskbarHost_ActivateOverflowFlyout_Original(pThis);
}

using CSecondaryTray_GetTrayWindow_t = HWND(WINAPI*)(void* pThis);
CSecondaryTray_GetTrayWindow_t CSecondaryTray_GetTrayWindow_Original;

using CSecondaryTray_InitModelAndHost_t = void(WINAPI*)(void* pThis, void* taskbarModel);
CSecondaryTray_InitModelAndHost_t CSecondaryTray_InitModelAndHost_Original;
void WINAPI CSecondaryTray_InitModelAndHost_Hook(void* pThis, void* taskbarModel) {
    if (IsModUnloading()) {
        CSecondaryTray_InitModelAndHost_Original(pThis, taskbarModel);

        return;
    }

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : CSecondaryTray::InitModelAndHost hook");
    }

    HWND taskbarWnd = CSecondaryTray_GetTrayWindow_Original(pThis);
    HMONITOR previousSecondaryInitMonitor = g_secondaryInitMonitor;
    int previousSecondaryInitMonitorIndex = g_secondaryInitMonitorIndex;
    bool previousInSecondaryTrayInit = g_inSecondaryTrayInit;
    bool previousSecondaryTaskbarHostPrimaryLike = g_secondaryTaskbarHostPrimaryLike;
    g_secondaryTaskbarHostPrimaryLike = false;

    if (taskbarWnd && ShouldApplyToTaskbar(taskbarWnd)) {
        g_secondaryInitMonitor = MonitorFromWindow(taskbarWnd, MONITOR_DEFAULTTONEAREST);
        g_secondaryInitMonitorIndex = GetMonitorIndex(g_secondaryInitMonitor);
        g_inSecondaryTrayInit = true;

        if (g_settings.enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : spoofing secondary tray init for monitor %d hwnd=0x%p model=0x%p", g_secondaryInitMonitorIndex, taskbarWnd, taskbarModel);
        }
    } else if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : secondary tray window unavailable before InitModelAndHost or skipped by settings hwnd=0x%p model=0x%p", taskbarWnd, taskbarModel);
    }

    CSecondaryTray_InitModelAndHost_Original(pThis, taskbarModel);

    g_secondaryInitMonitor = previousSecondaryInitMonitor;
    g_secondaryInitMonitorIndex = previousSecondaryInitMonitorIndex;
    g_inSecondaryTrayInit = previousInSecondaryTrayInit;
    bool primaryLikeHost = g_secondaryTaskbarHostPrimaryLike;
    g_secondaryTaskbarHostPrimaryLike = previousSecondaryTaskbarHostPrimaryLike;

    if (g_settings.enableVerboseLogging && primaryLikeHost) {
        Wh_Log_safe(L"taskbar-multi-tray : secondary tray init used primary-like TaskbarHost flag");
    }

    if (!taskbarWnd) {
        taskbarWnd = CSecondaryTray_GetTrayWindow_Original(pThis);
    }
    if (!taskbarWnd || !ShouldApplyToTaskbar(taskbarWnd)) {
        return;
    }

    XamlRoot xamlRoot = GetSecondaryTaskbarXamlRoot(taskbarWnd);

    if (!xamlRoot) {
        Wh_Log_safe(L"taskbar-multi-tray : secondary XamlRoot not found");

        return;
    }

    ApplyStyle(xamlRoot, taskbarWnd);
}

HRESULT WINAPI TrayUI__SetStuckMonitor_Hook(void* pThis, HMONITOR monitor) {
    if (IsModUnloading() && !g_restoringNativeTaskbars) {
        return TrayUI__SetStuckMonitor_Original(pThis, monitor);
    }

    HMONITOR targetMonitor = GetRealPrimaryTrayTargetMonitor();

    if (targetMonitor) {
        if (g_settings.enableVerboseLogging) {
            int targetIndex = g_restoringNativeTaskbars
                ? GetMonitorIndex(targetMonitor)
                : g_runtimePrimaryTrayMonitor
                    ? g_runtimePrimaryTrayMonitorIndex
                    : g_sharedProxyState.runtimePrimaryTrayMonitor
                        ? g_sharedProxyState.runtimePrimaryTrayMonitorIndex
                        : !g_settings.selectedMonitorOrder.empty()
                            ? g_settings.selectedMonitorOrder.front()
                            : GetMonitorIndex(targetMonitor);
            Wh_Log_safe(
                L"taskbar-multi-tray : %s real primary tray to monitor %d",
                g_restoringNativeTaskbars ? L"restoring" : L"moving",
                targetIndex
            );
        }

        monitor = targetMonitor;
    } else if (g_settings.monitorMode == MonitorMode::All && g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : monitorMode=all can't duplicate the real primary tray model; applying XAML visibility/layout only");
    }

    return TrayUI__SetStuckMonitor_Original(pThis, monitor);
}

bool HookTaskbarDllSymbols() {
    HMODULE module = LoadLibraryEx(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

    if (!module) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to load taskbar.dll");

        return false;
    }

    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        {
            {LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
            &CTaskBand_ITaskListWndSite_vftable,
        },
        {
            {LR"(const CSecondaryTaskBand::`vftable'{for `ITaskListWndSite'})"},
            &CSecondaryTaskBand_ITaskListWndSite_vftable,
        },
        {
            {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
            &CTaskBand_GetTaskbarHost_Original,
        },
        {
            {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CSecondaryTaskBand::GetTaskbarHost(void)const )"},
            &CSecondaryTaskBand_GetTaskbarHost_Original,
        },
        {
            {LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
            &TaskbarHost_FrameHeight_Original,
        },
        {
            {
                LR"(public: void __cdecl TaskbarHost::ActivateOverflowFlyout(void))",
                LR"(?ActivateOverflowFlyout@TaskbarHost@@QEAAXXZ)",
            },
            &TaskbarHost_ActivateOverflowFlyout_Original,
            TaskbarHost_ActivateOverflowFlyout_Hook,
        },
        {
            {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
            &std__Ref_count_base__Decref_Original,
        },
        {
            {LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"},
            &TrayUI_StartTaskbar_Original,
            TrayUI_StartTaskbar_Hook,
        },
        {
            {
                LR"(public: __cdecl TaskbarHost::TaskbarHost(struct ITrayComponentHost *,struct winrt::WindowsUdk::UI::Shell::TaskbarModel,struct winrt::WindowsUdk::UI::Shell::TaskbarControllerExtension const &,struct HMONITOR__ *,struct HWND__ *,bool,enum COLORTHEME,class CSecondaryTray *))",
                LR"(??0TaskbarHost@@QEAA@PEAUITrayComponentHost@@UTaskbarModel@Shell@UI@WindowsUdk@winrt@@AEBUTaskbarControllerExtension@3456@PEAUHMONITOR__@@PEAUHWND__@@_NW4COLORTHEME@@PEAVCSecondaryTray@@@Z)",
            },
            &TaskbarHost_Ctor_Original,
            TaskbarHost_Ctor_Hook,
        },
        {
            {
                LR"(private: void __cdecl SystemTrayHost::TryLoadNotificationAreaFrameFromXamlExtension(void))",
                LR"(?TryLoadNotificationAreaFrameFromXamlExtension@SystemTrayHost@@AEAAXXZ)",
            },
            &SystemTrayHost_TryLoadNotificationAreaFrameFromXamlExtension_Original,
            SystemTrayHost_TryLoadNotificationAreaFrameFromXamlExtension_Hook,
        },
        {
            {LR"(public: long __cdecl TrayUI::_SetStuckMonitor(struct HMONITOR__ *))"},
            &TrayUI__SetStuckMonitor_Original,
            TrayUI__SetStuckMonitor_Hook,
        },
        {
            {LR"(public: virtual struct HWND__ * __cdecl CSecondaryTray::GetTrayWindow(void))"},
            &CSecondaryTray_GetTrayWindow_Original,
        },
        {
            {LR"(public: virtual void __cdecl CSecondaryTray::InitModelAndHost(struct winrt::WindowsUdk::UI::Shell::TaskbarModel))"},
            &CSecondaryTray_InitModelAndHost_Original,
            CSecondaryTray_InitModelAndHost_Hook,
        },
    };

    if (!HookSymbols(module, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks))) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to hook taskbar.dll symbols");

        return false;
    }

    Wh_Log_safe(L"taskbar-multi-tray : hooked taskbar.dll symbols");

    auto hookOptional = [&](PCWSTR label, WindhawkUtils::SYMBOL_HOOK* hook) {
        if (HookSymbols(module, hook, 1)) {
            Wh_Log_safe(L"taskbar-multi-tray : hooked optional taskbar symbol: %s", label);

            return;
        }

        Wh_Log_safe(L"taskbar-multi-tray : optional taskbar symbol unavailable: %s", label);
    };

    // taskbar.dll
    WindhawkUtils::SYMBOL_HOOK setNotificationAreaIconManager2Hook[] = {
        {
            {
                LR"(public: void __cdecl winrt::WindowsUdk::UI::Shell::implementation::TaskbarModel::SetNotificationAreaIconManager2(class std::shared_ptr<class NotificationAreaIconManager2>))",
                LR"(?SetNotificationAreaIconManager2@TaskbarModel@implementation@Shell@UI@WindowsUdk@winrt@@QEAAXV?$shared_ptr@VNotificationAreaIconManager2@@@std@@@Z)",
            },
            &TaskbarModel_SetNotificationAreaIconManager2_Original,
            TaskbarModel_SetNotificationAreaIconManager2_Hook,
        },
    };
    hookOptional(L"TaskbarModel::SetNotificationAreaIconManager2", setNotificationAreaIconManager2Hook);

    // taskbar.dll
    WindhawkUtils::SYMBOL_HOOK getNotificationAreaPromotedIconsHook[] = {
        {
            {
                LR"(public: virtual int __cdecl winrt::impl::produce<struct winrt::WindowsUdk::UI::Shell::implementation::TaskbarModel,struct winrt::WindowsUdk::UI::Shell::ITaskbarModel3>::get_NotificationAreaPromotedIcons(void * *))",
                LR"(?get_NotificationAreaPromotedIcons@?$produce@UTaskbarModel@implementation@Shell@UI@WindowsUdk@winrt@@UITaskbarModel3@3456@@impl@winrt@@UEAAHPEAPEAX@Z)",
            },
            &TaskbarModel_GetNotificationAreaPromotedIcons_Original,
            TaskbarModel_GetNotificationAreaPromotedIcons_Hook,
        },
    };
    hookOptional(L"TaskbarModel::get_NotificationAreaPromotedIcons", getNotificationAreaPromotedIconsHook);

    // taskbar.dll
    WindhawkUtils::SYMBOL_HOOK getNotificationAreaOverflowIconsHook[] = {
        {
            {
                LR"(public: virtual int __cdecl winrt::impl::produce<struct winrt::WindowsUdk::UI::Shell::implementation::TaskbarModel,struct winrt::WindowsUdk::UI::Shell::ITaskbarModel6>::get_NotificationAreaOverflowIcons(void * *))",
                LR"(?get_NotificationAreaOverflowIcons@?$produce@UTaskbarModel@implementation@Shell@UI@WindowsUdk@winrt@@UITaskbarModel6@3456@@impl@winrt@@UEAAHPEAPEAX@Z)",
            },
            &TaskbarModel_GetNotificationAreaOverflowIcons_Original,
            TaskbarModel_GetNotificationAreaOverflowIcons_Hook,
        },
    };
    hookOptional(L"TaskbarModel::get_NotificationAreaOverflowIcons", getNotificationAreaOverflowIconsHook);

    // taskbar.dll
    WindhawkUtils::SYMBOL_HOOK addIconToVisibleCollectionHook[] = {
        {
            {
                LR"(private: void __cdecl NotificationAreaIconManager2::AddIconToVisibleCollection(struct winrt::WindowsUdk::UI::Shell::implementation::NotificationAreaIcon2 *))",
                LR"(?AddIconToVisibleCollection@NotificationAreaIconManager2@@AEAAXPEAUNotificationAreaIcon2@implementation@Shell@UI@WindowsUdk@winrt@@@Z)",
            },
            &NotificationAreaIconManager2_AddIconToVisibleCollection_Original,
            NotificationAreaIconManager2_AddIconToVisibleCollection_Hook,
        },
    };
    hookOptional(L"NotificationAreaIconManager2::AddIconToVisibleCollection", addIconToVisibleCollectionHook);

    // taskbar.dll
    WindhawkUtils::SYMBOL_HOOK removeIconFromVisibleCollectionHook[] = {
        {
            {
                LR"(private: void __cdecl NotificationAreaIconManager2::RemoveIconFromVisibleCollection(struct winrt::WindowsUdk::UI::Shell::implementation::NotificationAreaIcon2 *))",
                LR"(?RemoveIconFromVisibleCollection@NotificationAreaIconManager2@@AEAAXPEAUNotificationAreaIcon2@implementation@Shell@UI@WindowsUdk@winrt@@@Z)",
            },
            &NotificationAreaIconManager2_RemoveIconFromVisibleCollection_Original,
            NotificationAreaIconManager2_RemoveIconFromVisibleCollection_Hook,
        },
    };
    hookOptional(L"NotificationAreaIconManager2::RemoveIconFromVisibleCollection", removeIconFromVisibleCollectionHook);

    // taskbar.dll
    WindhawkUtils::SYMBOL_HOOK shellNotifyIconHook[] = {
        {
            {
                LR"(public: bool __cdecl NotificationAreaIconManager2::ShellNotifyIcon(struct _TRAYNOTIFYDATAW * __ptr64 const))",
                LR"(?ShellNotifyIcon@NotificationAreaIconManager2@@QEAA_NQEAU_TRAYNOTIFYDATAW@@@Z)",
            },
            &NotificationAreaIconManager2_ShellNotifyIcon_Original,
            NotificationAreaIconManager2_ShellNotifyIcon_Hook,
        },
    };
    hookOptional(L"NotificationAreaIconManager2::ShellNotifyIcon", shellNotifyIconHook);

    return true;
}

bool HookTwinuiPcshellSymbols() {
    HMODULE module = LoadLibraryEx(L"twinui.pcshell.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

    if (!module) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to load twinui.pcshell.dll");

        return false;
    }

    // twinui.pcshell.dll
    WindhawkUtils::SYMBOL_HOOK twinuiPcshellHooks[] = {
        {
            {LR"(public: bool __cdecl ImmersiveMonitorHelper::ConnectToMonitor(struct HWND__ *,struct tagPOINT))"},
            &ImmersiveMonitorHelper_ConnectToMonitor_Original,
            ImmersiveMonitorHelper_ConnectToMonitor_Hook,
        },
        {
            {LR"(public: long __cdecl ImmersiveMonitorHelper::AdjustMonitorConnectedIfNeeded(void))"},
            &ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Original,
            ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Hook,
        },
    };

    if (!HookSymbols(module, twinuiPcshellHooks, ARRAYSIZE(twinuiPcshellHooks))) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to hook twinui.pcshell.dll symbols");

        return false;
    }

    Wh_Log_safe(L"taskbar-multi-tray : hooked twinui.pcshell.dll symbols");

    return true;
}

BOOL Wh_ModInit() {
    g_modUnloading = 0;
    g_activeFlyoutRedirectionSuppressionDepth = 0;
    g_targetProcess = GetTargetProcess();
    LoadSettings();
    Wh_Log_safe(L"taskbar-multi-tray : init target=%s", TargetProcessToString(g_targetProcess));

    if (IsExplorerTarget() && !HookTaskbarDllSymbols()) {
        return FALSE;
    }

    if (!HookTwinuiPcshellSymbols()) {
        Wh_Log_safe(L"taskbar-multi-tray : immersive flyout monitor hook not installed");
    }

    WindhawkUtils::SetFunctionHook(MonitorFromPoint, MonitorFromPoint_Hook, &MonitorFromPoint_Original);
    WindhawkUtils::SetFunctionHook(MonitorFromRect, MonitorFromRect_Hook, &MonitorFromRect_Original);
    WindhawkUtils::SetFunctionHook(MonitorFromWindow, MonitorFromWindow_Hook, &MonitorFromWindow_Original);
    WindhawkUtils::SetFunctionHook(SetWindowPos, SetWindowPos_Hook, &SetWindowPos_Original);
    WindhawkUtils::SetFunctionHook(MoveWindow, MoveWindow_Hook, &MoveWindow_Original);
    WindhawkUtils::SetFunctionHook(SetWindowPlacement, SetWindowPlacement_Hook, &SetWindowPlacement_Original);
    WindhawkUtils::SetFunctionHook(DeferWindowPos, DeferWindowPos_Hook, &DeferWindowPos_Original);
    WindhawkUtils::SetFunctionHook(EnumDisplayDevicesW, EnumDisplayDevicesW_Hook, &EnumDisplayDevicesW_Original);
    WindhawkUtils::SetFunctionHook(DispatchMessageW, DispatchMessageW_Hook, &DispatchMessageW_Original);

    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log_safe(L"taskbar-multi-tray : after init");

    if (IsExplorerTarget()) {
        ApplySettings();
        QueueDeferredApplySettings();
    }
}

void Wh_ModBeforeUninit() {
    Wh_Log_safe(L"taskbar-multi-tray : before uninit");
    g_modUnloading = 1;
    CancelDeferredApplySettings();

    if (!IsExplorerTarget()) {
        ClearProxyRuntimeState();
        ClearCachedXamlBindings();

        return;
    }

    HWND taskbarWnd = FindCurrentProcessTaskbarWnd();

    if (!taskbarWnd) {
        ClearProxyRuntimeState();
        ClearCachedXamlBindings();

        return;
    }

    ClearProxyRuntimeState();
    ClearCachedXamlBindings();
    g_lastProxyPostWnd = nullptr;
    g_lastProxyPostFlyout = 0;
    g_lastProxyPostTick = 0;

    g_restoringNativeTaskbars = true;
    g_nativePrimaryRestoreMonitor = GetActualMonitorFromWindow(taskbarWnd, MONITOR_DEFAULTTONEAREST);

    if (g_settings.enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : native restore target monitor=%d", GetMonitorIndex(g_nativePrimaryRestoreMonitor));
    }

    RunFromWindowThread(taskbarWnd, RemoveTaskbarSubclassesFromTaskbarThread, nullptr);
    NotifyTaskbarDisplayChange(taskbarWnd);
    RunFromWindowThread(taskbarWnd, RestoreNativeTaskbarsFromTaskbarThread, nullptr);
    RunFromWindowThread(taskbarWnd, RemoveTaskbarSubclassesFromTaskbarThread, nullptr);

    g_restoringNativeTaskbars = false;
    g_nativePrimaryRestoreMonitor = nullptr;
}

void Wh_ModSettingsChanged() {
    Wh_Log_safe(L"taskbar-multi-tray : settings changed");
    LoadSettings();
    CancelDeferredApplySettings();
    ClearProxyRuntimeState();
    ClearCachedXamlBindings();
    g_lastProxyPostWnd = nullptr;
    g_lastProxyPostFlyout = 0;
    g_lastProxyPostTick = 0;

    if (IsExplorerTarget()) {
        ApplySettings();
        QueueDeferredApplySettings();
    }
}
