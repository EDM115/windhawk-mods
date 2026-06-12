// ==WindhawkMod==
// @id              taskbar-multi-tray
// @name            Taskbar multi-tray
// @description     Windows 11 taskbar tray/control-center visibility controls for all or selected monitors
// @version         1.1.0
// @author          EDM115
// @github          https://github.com/EDM115
// @twitter         https://twitter.com/_edm115
// @homepage        https://edm115.dev/
// @donateUrl       https://paypal.me/8EDM115
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
- [ ] Some machines might show empty space between tray icons and the clock. On secondary monitors this can also show as a clipped notification bell and a missing tray overflow button, likely tied to DPI/layout measurements. In the same category, compatibility with extra items in this taskbar's area (language selector, emoji icon, stylus menu, other shortcuts) isn't guaranteed.
- [ ] Reusing primary tray XAML bindings remains structurally dependent on internal XAML element names/classes, which can drift across Windows updates. Since 1.1.0 every apply/binding path is wrapped in exception boundaries, so drift degrades to a logged skip of that pass instead of risking an Explorer crash, but the mod cannot populate surfaces whose structure it no longer recognizes.

#### Fixed bugs (in antichronological order)
- [x] A XAML exception thrown while walking taskbar children (visual-tree or `Name()` calls on an element disconnected mid-walk, internal structure drift after a Windows update) escaped the apply pass uncaught into Explorer's window procedure, which can kill Explorer. Every apply/restore pass now runs inside an exception boundary that logs the HRESULT and skips that pass.
- [x] A flyout context expiring (or dying with its unplugged monitor) while another click was arming a new context could wipe the freshly armed context in the cross-process shared state, sending the new flyout to the wrong monitor, and `ShortenActiveFlyoutMonitorContext` could likewise republish stale context data over a newer generation. Both writes are now generation-guarded and an expiry no longer clears anything when the seqlock read raced a writer mid-arm.
- [x] `GetMonitorIndex`/`GetMonitorByIndex` ran a full `EnumDisplayMonitors` pass per query (behind settings targeting, context arming, and most verbose logs). The monitor numbering is now cached behind a slim reader/writer lock with a 1s TTL, invalidated eagerly on `WM_DISPLAYCHANGE` and when an armed context's monitor handle dies.
- [x] `ApplyStyle` did one full XAML child walk per element name and ran up to a dozen synchronous `UpdateLayout` passes per taskbar, re-dirtying layout on every pass because `Width(NaN)` over NaN registers as a property change. Elements are now collected in a single child walk, every property write is read-compare-write, and one batched layout update runs per taskbar only when something actually changed, making steady-state apply passes true no-ops. The promoted-icon width is also measured after that batched layout, so the click hit-test gets this pass's geometry instead of the previous one's.
- [x] Every monitor or window-placement query in both processes paid for a full cross-process seqlock snapshot (plus, since 1.0.9, a monitor-liveness check) even with no flyout context armed, and the redirection hooks re-ran that resolution up to five times per call through per-field accessors while heap-allocating class-name strings. A two-read fast path settles the idle case, hooks resolve the context once and read the synced locals, and hot-path class names use stack buffers.
- [x] The mod can still fail to fully apply when Explorer starts before the taskbar/XAML tree is ready. Version 1.0.3 extends deferred startup retries, but a later taskbar/XAML lifecycle hook would be stronger. Reproduce by enabling the mod and restarting `explorer.exe`, then testing again after reboot. This seems to only apply at boot time, as restarts of the explorer process apply correctly the mod.
- [x] Behavior when connecting/disconnecting monitors isn't guaranteed. Since 1.0.9 flyout contexts drop dead monitor handles and the next open re-resolves from the cursor, and 1.1.0 invalidates the cached monitor numbering on display changes, but the taskbar styling itself still depends on Windows recreating the taskbars.
- [x] Flyouts (mostly the hidden tray icons one, more rarely the control center) could open on the wrong monitor when a click never armed a clicked-monitor context : the right-edge hit-test depends on the promoted-icon width measured during the last apply pass, so any tray-icon change since then shifted the chevron region (the control-center region uses fixed offsets only, hence its relative rarity), and pen/touch input bypasses the subclass left-click path entirely. Unredirected fresh flyout opens are now rescued at placement time from the live cursor position (the `start-menu-open-location` strategy), scoped to not-yet-visible overflow/control-center popups while the cursor sits on a managed taskbar whose monitor differs from the requested placement. Armed contexts are also validated against the live monitor topology (the PowerToys Command Palette pattern), so a display unplug drops the context instead of forcing dead-handle geometry.
- [x] Windows builds one shared per-glyph control-center menu locked to the first monitor it opens on. The mod shows it on other monitors through a per-island proxy by briefly moving the menu's items across, so only one control-center menu can be open at a time and a fast right-click on a second monitor immediately after closing one on another can momentarily show an empty menu until the items finish returning home.
- [x] On rare occasions, one of the taskbars can crash, in which case it restarts automatically. Versions 1.0.1 to 1.0.7 remove several confirmed contributors, most recently the primary-like TaskbarHost hijack behind the control-center right-click crashes, any remaining crash needs fresh offsets/logs.
- [x] Right-clicking control-center icons crashed every taskbar except the single current menu-owner island, with zero mod log entries at click time. Two ownership hijacks stacked up : forcing primary-like `TaskbarHost` construction during secondary tray init (0.11/0.12) rewired the singleton tray model to a spoofed host at startup, and the mod's own shared-ItemsSource push had the same effect at the XAML level : the per-glyph context-menu state follows the most recent ItemsSource attach, so only the last-bound island could open the menus while every other taskbar (including the fully native primary) crashed touching that state from a foreign island. Right-tap gestures were irrelevant (menus kept opening with right-tap disabled, crashes persisted with gestures off), and click-time transfer proved structurally too late : with re-attach on the right-click notification, menus worked on whichever taskbar was clicked first, but the lethal press on the next monitor died inside the island's input processing before the taskbar window received any message (breadcrumbless crashes). The primary-like forcing and init spoofing are removed, and menu ownership now follows the pointer : hovering or right-pressing a non-owner control-center region re-attaches that taskbar's items source before the island processes the press, mid-press input never re-attaches, ownership returns to the native primary on restore/unload, and breadcrumbs log the tracked owner. The last crash layer was the lazily created per-glyph MenuFlyout staying rooted on the island where it first opened : showing a cached flyout anchored to a foreign XamlRoot kills Explorer, and the instance lives in the view-model where no tree walk can reach it (found=0 logged everywhere). The menus are therefore intercepted at the exact crashing call : the MenuFlyout/Flyout ShowAt ABI vtable slots are patched on the taskbar thread, and any flyout shown at a target inside a ControlCenterButton subtree is captured and re-rooted onto the target's island via FlyoutBase.XamlRoot before the native ShowAt runs (the documented island pattern), with ownership transfers and restore/unload re-rooting the captured instances as well. An already-shown flyout's XamlRoot is permanently locked to its origin island, so the menu cannot be moved : the mod keeps one proxy MenuFlyout per island and when a locked menu is asked to open elsewhere, moves the cached menu's items (with their native handlers) into that island's proxy and shows the proxy there, suppressing the crashy native ShowAt. Items are returned to their home menu before the next interaction and on unload, so the per-glyph menus open natively on every monitor with no crash.
- [x] Right-clicking control-center icons on monitors 2/3 still crashed every taskbar in 1.0.5, and always 1.8-3s *after* the click rather than at click time. Arming the flyout context called `SetTimer(taskbarWnd, 3, ...)` on Explorer's own `Shell_TrayWnd`/`Shell_SecondaryTrayWnd` windows : a tiny timer id that can silently replace a native Explorer timer, which the mod's subclass then swallowed and killed when it fired : exactly within the observed crash window on every crash, while the redirections themselves were identity no-ops on the primary monitor (3). The eager-clear timer is removed entirely (contexts already expire lazily via their tick deadline), the cross-thread timer-state vector is gone with it, and the deferred-apply timer moved to a mod-unique id so no native taskbar timer can be stomped or swallowed.
- [x] Right-clicking control-center icons (network, volume, battery) on any non-laptop taskbar crashed every taskbar, and the date-time right-click menu was swallowed or crashed there too. The 1.0.4 suppress-and-synthesize guard swallowed the raw second-button pointer messages and replayed a synthetic `WM_CONTEXTMENU`, feeding XAML an unmatched pointer stream. The guard is removed and every taskbar now uses the fully native right-click path that was already stable on the laptop taskbar, the mod only arms the clicked-monitor context and consumes the duplicate parent-level taskbar notification.
- [x] Clicking the date-time/notification area refreshed all taskbars, most visibly on monitors with a different resolution/DPI/zoom level, because the click was proxied through synthetic `Win+N` input and a temporary real-tray move. The proxy is removed entirely, Windows already opens the notification/date-time flyout on the clicked monitor natively. `Win+A` proxy is removed too.
- [x] Flyouts (control center, hidden tray icons) could still open on the wrong screen when a stale clicked-taskbar context survived a later interaction. Active flyout contexts now resolve the destination monitor from the live cursor position, like the `start-menu-open-location` mod does.
- [x] Active native-flyout `MonitorFromRect` forcing was broadened after 1.0.0, causing full monitor rectangles from every display to be reported as the clicked secondary monitor during non-primary flyout opens. The last-known-good target-side filtering is restored so only ambiguous rectangles or rectangles already on the clicked monitor are forced.
- [x] Removing primary `ControlCenterButton` binding reuse while chasing the crash left secondary control-center containers empty and non-clickable. Heap-backed binding reuse is restored for content and click behavior, while the rect forcing rollback and removed mouse hook remain.
- [x] A scoped child-HWND right-click guard added during 1.0.4 testing could crash-loop Explorer at startup before the taskbar XAML tree was ready. That guard is removed, and the control-center XAML subtree is no longer subclassed.
- [x] Repeated cross-monitor control-center clicks and right-click attempts could crash Explorer after the 1.0.1 path changes. The unstable broad native-flyout rect forcing is reverted to target-side filtering, the experimental taskbar mouse hook is removed, and right-click now suppresses only the unsafe raw second-button pointer-down and posts the native context menu at pointer release, preserving the native network, volume, and battery menus.
- [x] Right-clicking the copied/native secondary control-center surface could still crash after 1.0.2 because context-menu messages from child/bridge HWNDs could bypass the taskbar subclass. `DispatchMessageW` now resolves the message point back to the target taskbar and prepares the native control-center monitor context before dispatching the menu path.
- [x] Native control-center monitor context could linger after the real `ControlCenterWindow` appeared, keeping `(0,0)` monitor queries pinned to an older clicked monitor. Control-center contexts now start shorter and shorten again after popup placement.
- [x] Deferred startup apply retries could stop too early when Explorer created the taskbar/XAML tree slowly at startup or reboot. The bounded retry schedule now extends across a longer Explorer startup window.
- [x] Right-clicking the copied/native secondary control-center surface could still crash the taskbar. Control-center context-menu messages now prepare the native monitor context and are allowed through to the native XAML menu path.
- [x] Native flyout contexts stayed active after same-monitor non-target taskbar clicks, which could move a later overflow flyout to the wrong monitor. Non-target taskbar clicks now clear any active flyout context.
- [x] Native control-center and hidden-tray contexts were armed on both mouse down and mouse up. They are now armed only on mouse down so the mouse-up path doesn't extend stale monitor state.
- [x] Active flyout `MonitorFromRect` queries could keep stale source-monitor rectangles and let flyouts appear on the wrong monitor. Rect monitor queries now force only ambiguous rectangles or geometry already on the clicked monitor, after a broader pinning attempt proved unstable.
- [x] Settings changes could race hooks reading `std::set`/`std::vector` settings. Settings are now immutable snapshots published atomically.
- [x] ShellHost could observe torn shared proxy state, mixing monitor, kind, DPI, anchor, and taskbar rectangle generations. Shared proxy state now uses an odd/even seqlock snapshot.
- [x] Taskbar-thread dispatch could block indefinitely through `SendMessage`. Cross-thread taskbar calls now use `SendMessageTimeout` with `SMTO_ABORTIFHUNG`.
- [x] Proxy notification flyout opening blocked the taskbar UI thread with `Sleep(120)`. The delayed open now uses a taskbar-window timer.
- [x] The dead proxy control-center mouse-click branch could poll for up to 800ms. The unreachable branch and mouse-click helpers were removed.
- [x] `HookTaskbarDllSymbols` resolved `taskbar.dll` symbols up to seven times. Core taskbar hooks are now resolved in a single `HookSymbols` call.
- [x] Diagnostic-only private hooks could prevent the whole mod from loading when symbols drifted. Required log-only hooks and optional diagnostic hook passes were removed from the load path.
- [x] Explorer primary-display spoofing affected unrelated shell callers outside flyout work. It is now scoped to active known flyouts or secondary-tray initialization.
- [x] The mod broadcast a synthetic shell-hook display-change message to every top-level window. The desktop-wide broadcast was removed.
- [x] Secondary promoted tray icon clicks could be treated as hidden-tray clicks. Hidden-tray hit testing is now narrowed to the chevron region.
- [x] Cached WinRT/XAML binding globals could release COM objects during process detach. Binding caches are now heap-backed and explicitly cleared during normal reload/settings paths.
- [x] Hot `SetWindowPos`/`DeferWindowPos` hooks did work before cheap flyout/class guards. They now bail before `GetWindowRect` when no relevant flyout context is active.
- [x] The string-setting helper carried a dead manual fallback branch. It now relies on Windhawk's setting defaults.
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
### 1.1.0
**Summary :** Performance release plus review fixes : monitor cache, single-pass batched apply, hot-hook fast paths, cross-process race guards, and apply-path exception hardening  
**Details :** A full review pass over the hot paths and the remaining known issues. **Monitor cache** : `GetMonitorIndex`/`GetMonitorByIndex` ran a full `EnumDisplayMonitors` enumeration per query, they now read a fixed-size snapshot behind a slim reader/writer lock with a 1-second TTL (covering ShellHost, which has no taskbar window), invalidated eagerly on `WM_DISPLAYCHANGE` in the taskbar subclass and when an armed context's monitor handle dies. **Idle fast path** : every `MonitorFrom*`, placement, and dispatch hook in both processes funnels through the active-context resolution, which always paid for a full seqlock snapshot (plus the 1.0.9 monitor-liveness check) even when no flyout context existed : two plain reads now settle the idle case, the redirection hooks resolve the context once per call and read the synced process-local fields instead of re-running the resolution through per-field accessors (those accessors are removed), and the hot paths use stack-buffer class names instead of heap-allocating `std::wstring`s. Hit testing fetches the window DPI once per pass instead of once per metric, and the control-center hover/dispatch pre-arm work is skipped entirely when the control-center surface is not managed. **Single-pass batched apply** : `ApplyStyle` did one full XAML child walk per element name and ran up to a dozen synchronous `UpdateLayout` passes per taskbar, and `Width(NaN)` written over NaN registered as a property change that re-dirtied layout on every pass (including each deferred startup retry). All elements are now collected in one child walk, every property write is read-compare-write, exact-width pins persist while an element stays collapsed and release once content lays out, and a single batched layout update runs per taskbar only when something actually changed, so steady-state passes are true no-ops. The promoted-icon width feeding the click hit-test is measured after that batched layout, giving the 1.0.9 cursor-rescue fix fresher data. **Race guards** : a context expiring (or being dropped for a dead monitor) while another click armed a new context could wipe the fresh context in the shared seqlock state, and `ShortenActiveFlyoutMonitorContext` could republish stale data over a newer generation : both writes are now generation-guarded, and an expiry no longer clears shared state when the snapshot read raced a writer mid-arm. **Hardening** : a XAML exception thrown while walking taskbar children (an element disconnected mid-walk, structure drift after a Windows update) previously escaped the apply pass uncaught into Explorer's window procedure : every apply/restore pass now runs inside an exception boundary that logs the HRESULT and degrades to a skipped pass, which is also the honest cap on the binding-fragility known issue : drift can no longer crash, but unrecognized structure cannot be populated. Return values of the apply passes now mean structural success, so the `apply failed` log no longer fires on no-op steady-state passes.

### 1.0.9
**Summary :** Fixed flyouts opening on the wrong monitor when no clicked-monitor context was armed, and hardened contexts against monitor hot-plug  
**Details :** The long-standing wrong-monitor flyout bug had two remaining causes. First, a chevron or control-center click could open the native flyout without ever arming a clicked-monitor context : the subclass right-edge hit-test depends on the promoted-icon width measured during the last apply pass, so any tray-icon change since then (sync clients, status icons appearing or disappearing) shifts the chevron region and the click falls through as a plain taskbar click, which even *clears* the previous context : this is why the hidden-icons flyout was affected far more often than the control center, whose region uses fixed offsets only. Pen and touch input never produces the subclass left-click messages at all. With no context armed nothing redirected the open, and the singleton flyout appeared wherever Windows natively placed it. Placement now gets a last-chance rescue modeled on the `start-menu-open-location` mod : when a known flyout popup (`TopLevelWindowForOverflowXamlIsland`, `ControlCenterWindow`) is placed while not yet visible and no context is armed, and the live cursor sits on a managed taskbar whose monitor differs from the requested placement monitor, a short cursor-derived context is armed on the spot (cursor point as the overflow anchor) and the placement is translated through the existing DPI-aware path. The rescue is scoped to fresh opens only, so the reverted 1.0.5 behavior where live-cursor targeting dragged delayed popups to whichever monitor the cursor reached cannot return, and keyboard-driven opens away from any taskbar stay fully native. Second, an armed context could outlive its monitor : after a display unplug the stored `HMONITOR` is dead, and forcing monitor queries to a dead handle breaks the shell's placement math. Borrowing the PowerToys Command Palette protection, every context read now validates the handle against the live monitor topology and drops the context when the monitor is gone, letting the cursor rescue re-resolve the following open instead.

### 1.0.8
**Summary :** Code-health release : dead code removed, every function documented, reviewer-oriented readme  
**Details :** No behavior changes. Removed machinery that the 1.0.7 investigation left behind dead : the runtime primary-tray publishing chain (it could only ever publish null after the notification/date-time proxy was dropped in 1.0.5), the captured-flyout re-root list (an already-shown flyout's XamlRoot is permanently locked, so those re-roots could never succeed once the per-island proxies took over), and the unused cursor-monitor helper. Trimmed the related log lines so every remaining log sits on a live code path, added Doxygen-style documentation to every function plus focused comments on the tricky parts (the cross-process seqlock, the TaskbarHost shared_ptr/XamlRoot extraction, the ShowAt vtable patching, the proxy item transfer), rebuilt the debugging filter list from the actual log strings, and rewrote the readme's How it works into a full technical walkthrough so reviewers don't have to reverse-engineer the source.

### 1.0.7
**Summary :** Made control-center right-click menus open on every taskbar by moving the menu ownership to the pointed island before the press  
**Details :** The first 1.0.7 step made tray and Control Center right-clicks a pure native fall-through : no pre-arming, forwarding, synthesizing, suppressing, or re-owning of right-click/context messages, with right-click input only clearing stale left-click flyout contexts. Fresh logs proved the crash survived that with **zero** mod log entries at click time, so the damage lived in startup state, not click handling. The second step removed the primary-like host hijack : since 0.11/0.12 the mod spoofed monitor/primary-display queries during secondary tray init and forced the `TaskbarHost` constructor's primary-like flag, attaching extra primary-like hosts to Windows' process-wide singleton tray model and rewiring it away from the real primary (confirmed by logs where the laptop taskbar natively carried the primary's exact item-source pointers without any mod binding pass, and stopped doing so once the spoof was removed). That alone wasn't enough : breadcrumbed logs then showed the surviving single-owner behavior follows the most recent shared-ItemsSource attach at the XAML level : the taskbar bound last opened menus fine **even with right-tap disabled**, while the fully native primary and the other copy kept crashing **even with gestures off**, killing the gesture theory entirely and proving the mod's own binding push is itself an ownership hijack. The per-glyph context-menu state can serve exactly one island at a time, which is also why the notification/date-time button never had this problem : Windows gives every taskbar its own items for that button natively. The third step re-attached the clicked taskbar's items source from the `WM_PARENTNOTIFY` right-button notification : menus then worked on whichever taskbar was right-clicked **first** (any monitor, all three glyphs, repeatedly), proving the re-attach transfer works : but right-clicking a second monitor still killed Explorer, and those lethal presses produced **no breadcrumb at all** while every surviving click logged one. A right press over a stale control-center region therefore dies inside the island's own input processing, before the taskbar window receives any message : left-clicks always survived on the same containers, so the right press eagerly touches the cached cross-island menu state at button down, and any click-time transfer is structurally too late. The fourth step pre-armed the ownership before the press could land : pure hover input headed into a non-owner taskbar island (mouse-move/pointer-update in the `DispatchMessageW` path, plus `WM_SETCURSOR` forwarded to the subclassed taskbar) re-attaches that taskbar's items source while no button is down, and the right-press dispatch itself is caught before the island's window procedure sees it as a last resort. Mid-press input never re-attaches (it would eat the active click), ownership is tracked per taskbar window (apply passes update it, `WM_NCDESTROY` clears it) and handed back to the native primary during restore/unload, context gestures stay fully native everywhere, and control-center right-clicks log breadcrumbs with the tracked owner state. Fresh logs confirmed the pre-arm chain works end to end (ownership moved via input dispatch on hover, the once-lethal press survived and logged contextOwner=1) and pinned the remaining crash at menu open : the first menu open lazily creates the per-glyph MenuFlyout, SystemTray caches the instance, and that flyout stays rooted on the island where it first opened : XAML kills the process when a cached flyout anchored to one XamlRoot is shown on another. Tree walks then proved the cached flyout lives in the view-model, not on any visual tree (found=0 logged on every taskbar even after menus had been used), and system XAML exposes no public handle to it (Popup.AssociatedFlyout is WinUI-only, this machine's Windows.UI.Xaml metadata tops out at IPopup4). The fix therefore intercepts the exact call that crashes : the ShowAt slots of the MenuFlyout/Flyout ABI vtables (IFlyoutBase slot 14, IFlyoutBase5 slot 12, layouts taken from the bundled C++/WinRT headers) are patched from the taskbar thread using throwaway instances to reach the class vtables shared by every flyout of those types. The hook is a pure pass-through unless the placement target sits inside a ControlCenterButton subtree, in which case the flyout is captured (heap-backed, like the binding caches) and re-rooted onto the target's island via FlyoutBase.XamlRoot right before the native ShowAt runs, so the menu opens locally no matter which island created it first. Ownership transfers also re-root all captured flyouts, restore/unload re-roots them onto the native primary and unpatches the vtable slots on the taskbar thread before releasing the references. Logs then showed the catch : a flyout's XamlRoot is settable only while it has never been shown (the first show on monitor 3 re-rooted fine, the reused per-glyph instance locked to monitor 3 then threw, and the native ShowAt against a monitor-2 target crashed). The re-root tries the full Hide -> clear-to-null -> set sequence, but logs proved the lock is permanent : the first show on a monitor re-roots fine, then the reused per-glyph instance is locked to that island forever (locked=3 on every later monitor) and the native ShowAt is suppressed, so no crash but no menu on the other monitors. The final piece works around the permanent lock with a per-island proxy : the mod keeps one MenuFlyout per taskbar island, rooted to that island from birth, and when a locked cached menu is asked to open on a foreign island the mod moves the cached menu's items (which carry their native network/volume/battery command handlers) into that island's proxy and shows the proxy there instead, suppressing the crashy native ShowAt. The borrowed items are handed back to their home menu at the start of the next control-center menu interaction (no Closed handler, so nothing mod-owned can outlive an unload), and unload returns every borrowed set before dropping the proxies and unpatching. Net result : the per-glyph control-center menus open natively on every monitor.

### 1.0.6
**Summary :** Fixed the delayed right-click crash on monitors 2/3 by removing taskbar-window timers  
**Details :** Fresh 1.0.5 logs confirmed the date-time fix and the native right-click menu, but control-center right-clicks on monitors 2 and 3 still killed Explorer : consistently 1.8 to 3 seconds *after* the click, never instantly, and never on monitor 1. On monitor 3 (the true primary) every monitor redirection the mod performs during that path is an identity no-op, so the redirection logic was ruled out. What remained was the eager context-clear timer : arming a flyout context ran `KillTimer`/`SetTimer` with timer id 3 directly on Explorer's `Shell_TrayWnd`/`Shell_SecondaryTrayWnd`. Tiny ids like that can collide with Explorer's own internal taskbar timers : `SetTimer` silently replaces the native timer, and the mod's subclass then swallowed and killed every id-3 tick. Both crash sessions died exactly within the window where that 1800 ms timer fired. The eager-clear timer and its cross-thread generation-tracking vector are removed completely : contexts already expire lazily through their tick deadline checked on every query, `ShortenActiveFlyoutMonitorContext` now only rewrites the deadline, and the deferred-apply retry timer moved from id 4 to a mod-unique id. The mod no longer creates or kills any low-id timer on Explorer's windows.

### 1.0.5
**Summary :** Fixed external-monitor right-click crashes, made date-time clicks fully native, and switched flyout targeting to the cursor monitor  
**Details :** Fresh 1.0.4 logs showed Explorer dying right after the external-taskbar right-click guard suppressed the second-button pointer stream and posted a synthetic `WM_CONTEXTMENU` : XAML received a pointer-down with no matching release and crashed, while the untouched laptop taskbar kept working. The suppress-and-synthesize guard is removed, every taskbar now takes the same fully native right-click path, with the mod only arming the clicked-monitor context and consuming the duplicate parent-level taskbar notification, so the per-glyph network/volume/battery menus and the date-time context menu open natively on every monitor. The notification/date-time proxy (synthetic `Win+N` input, real-tray moves, restore timers) is removed because Windows already opens that flyout on the clicked monitor natively, which also removes the all-taskbar refresh after clicking the clock on mixed-DPI monitors. Active flyout contexts now resolve their destination monitor from the live cursor position (the same approach as the `start-menu-open-location` mod) instead of the stored clicked-taskbar snapshot, so stale contexts can no longer drag a flyout to the wrong screen.

### 1.0.4
**Summary :** Restored safe native-flyout monitor forcing and taskbar control-center content  
**Details :** Fresh 1.0.3 logs showed non-primary flyout opens forcing `MonitorFromRect` for full monitor rectangles from every display to the clicked secondary monitor, followed by the primary taskbar crashing. The active native-flyout rect logic is back to the last-known-good shape from 1.0.0, forcing only ambiguous rectangles or rectangles already on the clicked monitor. Later logs showed secondary `ControlCenterButton` containers were visible and hoverable but empty and non-clickable after binding reuse was disabled, so heap-backed primary control-center binding reuse is restored. Right-click no longer posts the proxy control-center flyout or refreshes taskbar state, it keeps native right-tap handling enabled, consumes only the broad parent taskbar notification, suppresses the unsafe raw second-button pointer-down, and posts the native context menu at pointer release so the network, volume, and battery menus can open on the clicked monitor. The experimental `WH_MOUSE` hook and startup child-HWND subclass attempt stay removed.

### 1.0.3
**Summary :** Hardened secondary control-center right-click handling and reduced stale flyout monitor forcing  
**Details :** Secondary control-center context-menu messages are now suppressed from `DispatchMessageW` even when a XAML child or bridge HWND receives the right-click path, native control-center monitor context starts shorter and is shortened again after `ControlCenterWindow` placement, and deferred startup retries now cover a longer Explorer startup/reboot window.

### 1.0.2
**Summary :** Fixed secondary control-center right-click crashes and stale flyout monitor contexts  
**Details :** Secondary control-center context menus are suppressed before native XAML menu handling, native flyout contexts are armed only on mouse down, any non-target taskbar click now clears stale flyout state even on the same monitor, and active `MonitorFromRect` queries were narrowed for stale source-monitor geometry. Version 1.0.4 restores target-side filtering after broad rect pinning proved unstable.

### 1.0.1
**Summary :** Reduced Explorer/ShellHost race, hang, and symbol-drift risk after upstream review  
**Details :** Settings are now immutable snapshots, shared proxy state uses a seqlock-style generation, taskbar-thread sends use hung-window timeouts, proxy notification opens use a timer instead of sleeping on the taskbar thread, log-only private taskbar hooks were removed from loading, Explorer primary-display spoofing is scoped to active flyouts/secondary initialization, the broad shell-hook broadcast was removed, hidden-tray hit testing was narrowed to the chevron, XAML binding caches avoid process-detach releases, and hot placement hooks bail before `GetWindowRect` when inactive.

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
This section is the technical map of the mod, following the architecture from injection to pixels.

### Process and hook layout
The mod is injected into `explorer.exe` (owner of every taskbar window and its XAML island) and `ShellHost.exe` (host of the `Win+A` control center and several flyouts on Windows 11 24H2+), `GetTargetProcess` picks the role at init. Explorer gets the `taskbar.dll` symbol hooks (`TrayUI::StartTaskbar`, `TrayUI::_SetStuckMonitor`, `CSecondaryTray::InitModelAndHost`, plus the `CTaskBand`/`CSecondaryTaskBand` accessors used to reach the XAML), resolved in a single `HookSymbols` pass so one drifted symbol fails the whole load instead of half-loading. Both processes get the user32 hooks (`MonitorFromPoint/Rect/Window`, `SetWindowPos`, `MoveWindow`, `SetWindowPlacement`, `DeferWindowPos`, `EnumDisplayDevicesW`, `DispatchMessageW`). ShellHost additionally hooks `twinui.pcshell.dll`'s `ImmersiveMonitorHelper` (optional, placement degrades gracefully without it).  
The two processes coordinate through a `.shared` PE section (`SharedProxyState`) guarded by a seqlock : writers flip a generation counter odd before touching the data and even after, readers retry until a stable even generation brackets their copy. The hooks are hot (every monitor query and window placement in two processes), so the no-context case is settled by a two-read fast path before any seqlock snapshot is taken, and once a context resolves, its synced process-local fields are read directly. Settings are immutable snapshots swapped atomically (`GetSettings`/`PublishSettings`), so hooks on any thread read coherent values without locks. Monitor numbers are one-based `EnumDisplayMonitors` positions for the current session, cached behind a short-TTL reader/writer snapshot that display changes invalidate eagerly.

### Reaching the taskbar XAML
Each taskbar window (`Shell_TrayWnd`, `Shell_SecondaryTrayWnd`) hosts a XAML island, and no public API maps an HWND to its `XamlRoot`. The mod follows Explorer's own objects instead : the window's `CTaskBand`/`CSecondaryTaskBand` is located by matching the `ITaskListWndSite` vtable pointer across the window-long slots, its `GetTaskbarHost()` returns a `std::shared_ptr<TaskbarHost>` by value, the offset of the hosted XAML element inside `TaskbarHost` is read out of the compiled prologue of `TaskbarHost::FrameHeight` (x64 `add rcx, imm8`/ARM64 `ldr x8, [x0, #imm]` encodings, so no hardcoded struct layout can silently rot), and the temporary `shared_ptr` is released through `std::_Ref_count_base::_Decref`. From the hosted element, `XamlRoot.Content -> SystemTray.SystemTrayFrame -> SystemTrayFrameGrid` is the container under which every tray surface the mod touches lives.

### The apply pass
`ApplyStyle` runs per taskbar, always on the taskbar thread : at startup from the `TrayUI::StartTaskbar` hook, on demand through `RunFromWindowThread` (a `WH_CALLWNDPROC` hook plus `SendMessageTimeout(SMTO_ABORTIFHUNG)`, so a hung Explorer cannot deadlock the caller), for hot-plugged taskbars from `CSecondaryTray::InitModelAndHost`, and through a bounded retry schedule on a mod-unique timer id for Explorer startups where the XAML tree appears late. The pass collects every tray element in a single child walk (`CollectTrayElements`), forces the configured surfaces (`NotifyIconStack`, `NotificationAreaIcons`, `ControlCenterButton`, `NotificationCenterButton`) visible, hit-testable and wide enough to lay out, resets non-selected taskbars to the default secondary look, and measures the promoted-icon-area width that click hit-testing uses later. Every property write is read-compare-write and one batched layout update runs per taskbar only when something actually changed, so a steady-state pass dirties nothing, and the width measurement happens after that layout so the hit-test always sees this pass's geometry. The whole pass sits inside an exception boundary : XAML structure drift degrades to a logged skip instead of letting an exception escape into Explorer's window procedure. The real-tray owner is processed first so its bindings are cached before any copy consumes them.

### Content : sharing the singleton bindings
Windows keeps exactly one real notification-area model, and the mod never duplicates the native icon manager or its `std::shared_ptr` ownership. Instead, the real-tray owner's elements act as a binding source : their `DataContext`/`ItemsSource`/(non-UIElement) `Content` are cached in heap-backed holders and applied to the matching elements of every other taskbar, which then render the singleton content through ordinary XAML data binding. `UIElement` content is never shared, a XAML element cannot live in two trees. In selected-monitor mode, the first configured monitor additionally becomes the preferred owner of the real tray surface : the `TrayUI::_SetStuckMonitor` hook retargets Explorer's own primary-taskbar placement logic there, re-triggered through Explorer's display-change message (`0x5B8`).

### Left clicks on copied surfaces : clicked-monitor contexts
A window subclass on every managed taskbar hit-tests left clicks against DPI-scaled fixed metrics measured from the right edge ([show desktop][notification/clock][control center][promoted icons][chevron]). A click on a copied control-center or hidden-icons surface arms a short-lived **flyout context** : monitor, flyout kind, tick deadline, chevron anchor point, taskbar rectangle and target DPI, stored locally and published through the shared seqlock so ShellHost redirects too. While a context is armed :
- `MonitorFrom*` queries resolve to the clicked monitor, gated by target-side filters so only ambiguous geometry, the clicked taskbar itself, and known flyout popups (`ControlCenterWindow`, `TopLevelWindowForOverflowXamlIsland`, the control-center windowed popup) are redirected
- `EnumDisplayDevicesW` reports the clicked display as the primary device
- `ImmersiveMonitorHelper` is connected to the clicked monitor's center
- placement calls for the known popups are rewritten : DPI-rescaled between source and target monitors, anchored to the captured chevron point for the overflow flyout, pinned to the armed taskbar's edge with a small gap, and clamped into the target work area

Contexts expire lazily through their tick deadline (no timers ever run on Explorer's taskbar windows for this), are shortened right after popup placement so they cannot linger, and are dropped by right-click input or left clicks outside the copied surfaces. Every context read validates the armed monitor handle against the live topology first, so a hot-plugged display drops the context instead of redirecting to a dead handle.  
When an open arrives with **no armed context** at all (the hit-test depends on measured icon widths that tray-icon changes can shift, and pen/touch input bypasses the subclass left-click path), the placement hooks arm a last-chance cursor-derived context : a known flyout popup placed while not yet visible, with the cursor on a managed taskbar whose monitor differs from the requested placement, is treated as a click on that taskbar (cursor point as the overflow anchor) and translated through the same path. The rescue never touches re-placements of an already-visible flyout, so a delayed popup cannot chase the cursor across monitors.

### Right clicks on control-center icons : the three-layer fix
The per-glyph network/volume/battery menus were the 1.0.7 crash saga, and each of the three layers below is load-bearing :
1. **Menu ownership follows the pointer**. The per-item context-menu state binds to whichever `ControlCenterButton` attached the shared `ItemsSource` most recently, and a right press over any other island kills Explorer inside the island's input processing, before the taskbar window receives a single message. The mod therefore re-attaches the hovered taskbar's items source while no button is down (mouse-move/pointer-update seen in the `DispatchMessageW` hook, `WM_SETCURSOR` forwarded to the subclassed taskbar), with the right-press dispatch itself as a last resort. Mid-press input never re-attaches, that would eat the active click.
2. **`ShowAt` is intercepted at the ABI vtable**. The menus are flyouts created lazily on first open and cached inside Windows' view-model, reachable through no tree walk and no public API (`Popup.AssociatedFlyout` is WinUI-only, system XAML tops out at `IPopup4`). Throwaway `MenuFlyout`/`Flyout` instances expose the class vtables shared by every instance, and their `IFlyoutBase::ShowAt` (slot 14) and `IFlyoutBase5::ShowAt` (slot 12) entries are patched on the taskbar thread, with duplicate-slot protection since flyout classes can share an implementation vtable. The hook is a pure pass-through unless the placement target sits inside a `ControlCenterButton` subtree.
3. **Per-island proxies beat the XamlRoot lock**. A flyout's `XamlRoot` is settable only before its first show, afterwards it is locked to its first island forever, and showing it anchored to another island is the exact crashing call. The mod keeps one proxy `MenuFlyout` per island, created with its `XamlRoot` set before first use. When a locked cached menu is asked to open on a foreign island, its items, which carry the native command handlers, are moved into that island's proxy, the proxy is shown at the original target, and the crashy native `ShowAt` is suppressed (`S_OK` without showing). Borrowed items are handed back to their home menu at the start of the next control-center interaction and on unload, with a guard against duplicating a menu Windows rebuilt in the meantime.

### What stays native
The notification/date-time button is untouched : Windows gives every taskbar its own items for it and already opens that flyout on the clicked monitor. Tray icon right clicks travel the notify-icon message pipeline to the owning apps, which is island-safe. ShellHost surfaces outside an armed flyout context, including Windows Search, keep their native placement.

### Lifecycle and safety
Unload is strictly ordered : the unloading flag flips every hook to pass-through, pending retries are cancelled, contexts and caches cleared, and then, on the taskbar's own thread, subclasses are removed, native styles and control-center menu ownership are restored, borrowed proxy items returned, and the vtable slots unpatched before the mod's code can vanish. No low timer ids are ever used on Explorer's windows (small ids collide with native taskbar timers, a confirmed past crash source). WinRT references live in heap-backed holders cleared only on normal reload/settings paths, never during process detach, so COM releases cannot run at an unsafe time.

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
Useful log filters when diagnosing issues :
- `settings monitorMode=` : the active settings dump
- `init target=`/`hooked taskbar.dll symbols`/`hooked twinui.pcshell.dll symbols`/`hooked FlyoutBase ShowAt vtable slots` : load and hook health
- `ApplyStyle hwnd=` : each per-taskbar apply pass with class, monitor and rectangle
- `element `/`width-collapsed` : XAML element state seen by the apply pass
- `tree `/`hidden-stack child` : bounded XAML dumps (`enableTreeDump` only)
- `cached primary`/`applied primary`/`binding monitor=`/`failed to share` : the singleton binding bridge
- `context gestures` : right-tap/holding state applied per surface
- `cached promoted icon width` : the measured icon-area width used by click hit-testing
- `resetting skipped monitor`/`restoring native primary tray state`/`restoring native taskbar XAML state`/`removed subclass/timers`/`native restore target monitor` : restore and unload paths
- `real primary tray to monitor`/`can't duplicate the real primary tray model` : the selected-mode real-tray move
- `secondary click` : subclass hit-testing of copied-surface clicks
- `flyout monitor context` : arming, clearing and shortening of clicked-monitor contexts (cursor-rescue arming included)
- `cursor rescue` : placement-time rescue of flyout opens that arrived with no armed context
- `anchored hidden tray flyout` : the chevron anchor captured for overflow placement
- `forcing MonitorFrom`/`not forcing MonitorFrom` : monitor-query redirection decisions
- `marking` : primary-display spoofing during active flyouts
- `forcing immersive flyout`/`redirecting ConnectToMonitor` : the twinui.pcshell placement steering
- `moving ` : popup placement translation with source/target rectangles and DPI
- `moved control center context ownership`/`control center right-click/context fall-through` : menu-ownership transfers and right-click breadcrumbs
- `control center flyout`/`result=locked viaProxy=` : the ShowAt hook decisions (native, re-rooted, or proxied)
- `scheduled deferred apply retry`/`deferred apply retry from taskbar timer` : the bounded startup retry schedule
- `failed` : every error path (including the apply-pass exception boundaries)

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
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/base.h>

#include <cwctype>
#include <functional>
#include <limits>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef WM_POINTERUPDATE
#define WM_POINTERUPDATE 0x0245
#endif
#ifndef WM_POINTERDOWN
#define WM_POINTERDOWN 0x0246
#endif
#ifndef WM_POINTERUP
#define WM_POINTERUP 0x0247
#endif
#ifndef POINTER_MESSAGE_FLAG_FIRSTBUTTON
#define POINTER_MESSAGE_FLAG_FIRSTBUTTON 0x00000010
#endif
#ifndef POINTER_MESSAGE_FLAG_SECONDBUTTON
#define POINTER_MESSAGE_FLAG_SECONDBUTTON 0x00000020
#endif
#ifndef IS_POINTER_FLAG_SET_WPARAM
#define IS_POINTER_FLAG_SET_WPARAM(wParam, flag) ((((DWORD)HIWORD(wParam)) & (flag)) == (flag))
#endif
#ifndef IS_POINTER_SECONDBUTTON_WPARAM
#define IS_POINTER_SECONDBUTTON_WPARAM(wParam) IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_SECONDBUTTON)
#endif

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

Settings g_defaultSettings;
void* volatile g_settingsSnapshot = &g_defaultSettings;

/// Returns the current immutable settings snapshot. Lock-free : a single atomic read of the published pointer gives hooks on any thread a coherent view. Falls back to defaults before the first publish.
const Settings& GetSettings() {
    void* snapshot = InterlockedCompareExchangePointer(
        reinterpret_cast<PVOID volatile*>(&g_settingsSnapshot),
        nullptr,
        nullptr
    );

    return *static_cast<const Settings*>(snapshot ? snapshot : &g_defaultSettings);
}

/// Atomically publishes a new immutable settings snapshot. The previous snapshot is intentionally leaked : a hook on another thread may still be reading it, and settings changes are rare enough that the bytes do not matter.
void PublishSettings(Settings settings) {
    // Keep old snapshots alive. Hooks can be running concurrently on Explorer or taskbar threads, so freeing a replaced std::set/std::vector would reintroduce the race this snapshot pattern is meant to remove.
    Settings* publishedSettings = new Settings(std::move(settings));
    InterlockedExchangePointer(
        reinterpret_cast<PVOID volatile*>(&g_settingsSnapshot),
        publishedSettings
    );
}

#define Wh_Log_safe(message, ...) \
    do { \
        if (GetSettings().enableVerboseLogging) { \
            Wh_Log(message __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while (false)

enum class TargetProcess {
    Explorer,
    ShellHost,
    Other,
};

TargetProcess g_targetProcess = TargetProcess::Other;

constexpr WPARAM kNativeControlCenter = 3;
constexpr WPARAM kNativeHiddenTray = 4;

constexpr DWORD kNativeFlyoutMonitorContextMs = 5000;
constexpr DWORD kNativeControlCenterMonitorContextMs = 1800;
constexpr DWORD kNativeFlyoutMonitorContextAfterPlacementMs = 650;
constexpr DWORD kDeferredApplyRetryDelaysMs[] = {750, 1500, 3000, 6000, 12000, 24000};

asm(".section .shared,\"dws\"\n");
#define SHARED_SECTION __attribute__((section(".shared")))

struct SharedProxyState {
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

/// Starts a seqlock write of the cross-process shared state : bumps the generation to an odd value so concurrent readers know the data is mid-write
LONG BeginSharedProxyStateWrite() {
    LONG generation = InterlockedIncrement(reinterpret_cast<volatile LONG*>(&g_sharedProxyState.proxyFlyoutGeneration));

    if ((generation & 1) == 0) {
        generation = InterlockedIncrement(reinterpret_cast<volatile LONG*>(&g_sharedProxyState.proxyFlyoutGeneration));
    }

    MemoryBarrier();

    return generation;
}

/// Completes a seqlock write : bumps the generation back to an even value and returns it (the value a reader must observe unchanged for a consistent snapshot)
LONG EndSharedProxyStateWrite() {
    MemoryBarrier();

    LONG generation = InterlockedIncrement(reinterpret_cast<volatile LONG*>(&g_sharedProxyState.proxyFlyoutGeneration));

    if (generation & 1) {
        generation = InterlockedIncrement(reinterpret_cast<volatile LONG*>(&g_sharedProxyState.proxyFlyoutGeneration));
    }

    return generation;
}

/// Seqlock read of the cross-process shared state into a local snapshot, retrying when a writer is active (odd generation) or the generation moved mid-copy
/// @return false when no consistent snapshot could be taken
bool ReadSharedProxyState(SharedProxyState* snapshot) {
    if (!snapshot) {
        return false;
    }

    for (int attempt = 0; attempt < 8; attempt++) {
        LONG generationBefore = g_sharedProxyState.proxyFlyoutGeneration;

        if (generationBefore & 1) {
            YieldProcessor();

            continue;
        }

        MemoryBarrier();
        *snapshot = g_sharedProxyState;
        MemoryBarrier();

        LONG generationAfter = g_sharedProxyState.proxyFlyoutGeneration;

        if (generationBefore == generationAfter && !(generationAfter & 1)) {
            snapshot -> proxyFlyoutGeneration = generationAfter;

            return true;
        }

        YieldProcessor();
    }

    return false;
}

void* CTaskBand_ITaskListWndSite_vftable;
void* CSecondaryTaskBand_ITaskListWndSite_vftable;

using CTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void* pThis, void** result);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;

using CSecondaryTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void* pThis, void** result);
CSecondaryTaskBand_GetTaskbarHost_t CSecondaryTaskBand_GetTaskbarHost_Original;

void* TaskbarHost_FrameHeight_Original;

using std__Ref_count_base__Decref_t = void(WINAPI*)(void* pThis);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

using TrayUI__SetStuckMonitor_t = HRESULT(WINAPI*)(void* pThis, HMONITOR monitor);
TrayUI__SetStuckMonitor_t TrayUI__SetStuckMonitor_Original;

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
volatile LONG g_deferredApplyGeneration = 0;
volatile LONG g_modUnloading = 0;
volatile LONG g_activeFlyoutRedirectionSuppressionDepth = 0;
HWND g_deferredApplyTimerWnd = nullptr;
LONG g_deferredApplyTimerGeneration = 0;
size_t g_deferredApplyRetryIndex = 0;

/// Adopts a shared-state snapshot into this process's local flyout-context globals. This is how ShellHost picks up contexts armed by Explorer.
void CopySharedProxySnapshotToLocal(const SharedProxyState& snapshot) {
    g_proxyFlyoutMonitor = snapshot.proxyFlyoutMonitor;
    g_proxyFlyoutMonitorIndex = snapshot.proxyFlyoutMonitorIndex;
    g_proxyFlyoutUntilTick = snapshot.proxyFlyoutUntilTick;
    g_proxyFlyoutGeneration = snapshot.proxyFlyoutGeneration;
    g_proxyFlyoutKind = snapshot.proxyFlyoutKind;
    g_proxyFlyoutTaskbarWnd = reinterpret_cast<HWND>(snapshot.proxyFlyoutTaskbarWnd);
    g_proxyFlyoutAnchorValid = snapshot.proxyFlyoutAnchorValid != 0;
    g_proxyFlyoutAnchorPoint = {snapshot.proxyFlyoutAnchorX, snapshot.proxyFlyoutAnchorY};
    g_proxyFlyoutTaskbarRectValid = snapshot.proxyFlyoutTaskbarRectValid != 0;
    g_proxyFlyoutTaskbarRect = {
        snapshot.proxyFlyoutTaskbarLeft,
        snapshot.proxyFlyoutTaskbarTop,
        snapshot.proxyFlyoutTaskbarRight,
        snapshot.proxyFlyoutTaskbarBottom,
    };
    g_proxyFlyoutTargetDpi = snapshot.proxyFlyoutTargetDpi;
}

struct CachedXamlBinding {
    winrt::Windows::Foundation::IInspectable dataContext = nullptr;
    winrt::Windows::Foundation::IInspectable itemsSource = nullptr;
    winrt::Windows::Foundation::IInspectable content = nullptr;
};

CachedXamlBinding& g_primaryNotificationAreaIconsBinding = *new CachedXamlBinding();
CachedXamlBinding& g_primaryNotifyIconStackBinding = *new CachedXamlBinding();
CachedXamlBinding& g_primaryNotifyIconStackChildBinding = *new CachedXamlBinding();
CachedXamlBinding& g_primaryNotifyIconStackListViewBinding = *new CachedXamlBinding();
CachedXamlBinding& g_primaryControlCenterButtonBinding = *new CachedXamlBinding();
// Taskbar window whose ControlCenterButton items were attached last. The per-glyph context-menu state follows the most recent ItemsSource attach, so only this island can open the native menus safely.
HWND g_controlCenterItemsOwnerTaskbarWnd = nullptr;

// Set once the FlyoutBase ShowAt vtable slots have been patched on the taskbar thread (see EnsureFlyoutShowAtVtableHooks)
volatile LONG g_flyoutShowAtHooksInstalled = 0;

// A flyout's XamlRoot locks permanently to the island of its first show (confirmed by logs : re-rooted on first show, locked=3 ever after). The cached per-glyph menus can therefore never be shown on another island. Instead, one proxy MenuFlyout is created per island, rooted there before its first show, and the cached menu's items are moved into the island's proxy for the duration of the interaction. The items keep their original handlers, and they are lazily returned to the cached flyout at the start of the next control-center menu interaction, so no Closed event subscription (and no unload hazard from mod-owned handlers) is needed.
struct ControlCenterProxyFlyoutEntry {
    winrt::Windows::UI::Xaml::Controls::MenuFlyout proxy{nullptr};
    winrt::Windows::UI::Xaml::XamlRoot proxyRoot{nullptr};
    winrt::Windows::UI::Xaml::Controls::MenuFlyout itemsSource{nullptr};
};

struct ControlCenterProxyFlyouts {
    std::vector<ControlCenterProxyFlyoutEntry> entries;
};

ControlCenterProxyFlyouts& g_controlCenterProxyFlyouts = *new ControlCenterProxyFlyouts();

struct TaskbarTrayMetrics {
    HWND hWnd = nullptr;
    double notificationAreaIconsWidth = 0.0;
};

std::vector<TaskbarTrayMetrics> g_taskbarTrayMetrics;

/// True once Wh_ModBeforeUninit has started. Every hook checks it to fall back to fully native behavior during teardown.
bool IsModUnloading() {
    return g_modUnloading != 0;
}

/// True while monitor-query redirection must stay off : during unload, and while the mod itself applies/restores taskbar XAML (see ActiveFlyoutRedirectionSuppressor)
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
bool IsRightClickOrContextMenuMessage(UINT uMsg, WPARAM wParam);
std::wstring GetWindowClassName(HWND hWnd);

using ImmersiveMonitorHelper_ConnectToMonitor_t = bool(WINAPI*)(void* pThis, HWND hWnd, POINT point);
ImmersiveMonitorHelper_ConnectToMonitor_t ImmersiveMonitorHelper_ConnectToMonitor_Original;

using ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_t = HRESULT(WINAPI*)(void* pThis);
ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_t ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Original;

/// Log-formatting helper for MonitorMode
PCWSTR MonitorModeToString(MonitorMode mode) {
    return mode == MonitorMode::Selected ? L"selected" : L"all";
}

/// Log-formatting helper for Components
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

/// Log-formatting helper for XAML Visibility
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

/// Identifies the injected host process (explorer.exe, ShellHost.exe, or other) from the process image name
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

/// Log-formatting helper for TargetProcess
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

/// True when running inside explorer.exe, the process that owns the taskbar windows and their XAML islands
bool IsExplorerTarget() {
    return g_targetProcess == TargetProcess::Explorer;
}

/// Finds this process's primary taskbar window (Shell_TrayWnd), validating both the class name and the owning process id
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

/// Returns the input without leading/trailing whitespace
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

/// Parses a comma-separated list of one-based monitor numbers, preserving the configured order and dropping duplicates and invalid tokens
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

/// Reads a Windhawk string setting into a std::wstring and frees the Windhawk buffer
std::wstring ReadStringSetting(PCWSTR settingName) {
    PCWSTR value = Wh_GetStringSetting(settingName);
    std::wstring result = value ? value : L"";

    if (value) {
        Wh_FreeStringSetting(value);
    }

    return result;
}

/// Loads every mod setting and publishes them as a fresh immutable snapshot
void LoadSettings() {
    Settings settings;
    std::wstring monitorMode = ReadStringSetting(L"monitorMode");
    settings.monitorMode = _wcsicmp(monitorMode.c_str(), L"selected") == 0 ? MonitorMode::Selected : MonitorMode::All;

    std::wstring selectedMonitors = ReadStringSetting(L"selectedMonitors");
    settings.selectedMonitorOrder = ParseMonitorListInOrder(selectedMonitors);
    settings.selectedMonitors = std::set<int>(
        settings.selectedMonitorOrder.begin(),
        settings.selectedMonitorOrder.end()
    );

    std::wstring components = ReadStringSetting(L"components");

    if (_wcsicmp(components.c_str(), L"tray") == 0) {
        settings.components = Components::Tray;
    } else if (_wcsicmp(components.c_str(), L"controlCenter") == 0) {
        settings.components = Components::ControlCenter;
    } else {
        settings.components = Components::All;
    }

    settings.enableVerboseLogging = Wh_GetIntSetting(L"enableVerboseLogging") != 0;
    settings.enableTreeDump = Wh_GetIntSetting(L"enableTreeDump") != 0;

    PublishSettings(settings);

    Wh_Log_safe(
        L"taskbar-multi-tray : settings monitorMode=%s selectedMonitors=%s "
        L"components=%s verbose=%d treeDump=%d",
        MonitorModeToString(settings.monitorMode),
        selectedMonitors.c_str(), ComponentsToString(settings.components),
        settings.enableVerboseLogging, settings.enableTreeDump
    );
}

constexpr int kMaxCachedMonitors = 16;
constexpr DWORD kMonitorCacheTtlMs = 1000;

// Monitor numbering cache. GetMonitorIndex/GetMonitorByIndex used to run a full EnumDisplayMonitors pass per query, and they sit behind settings targeting, context arming, hit-test logging, and the cursor rescue. The cache keeps the EnumDisplayMonitors order behind a slim reader/writer lock and expires by TTL (covers ShellHost, which has no taskbar window to receive display-change messages), with eager invalidation on WM_DISPLAYCHANGE and when an armed context's monitor handle dies.
SRWLOCK g_monitorCacheLock = SRWLOCK_INIT;
HMONITOR g_monitorCacheEntries[kMaxCachedMonitors];
int g_monitorCacheCount = 0;
DWORD g_monitorCacheTick = 0;
bool g_monitorCacheValid = false;

/// Drops the cached monitor list so the next query re-enumerates against the live topology
void InvalidateMonitorCache() {
    AcquireSRWLockExclusive(&g_monitorCacheLock);
    g_monitorCacheValid = false;
    ReleaseSRWLockExclusive(&g_monitorCacheLock);
}

/// Uncached EnumDisplayMonitors pass into a fixed buffer, in the session-stable order the one-based settings numbers refer to
int EnumerateMonitorsUncached(HMONITOR* entries, int capacity) {
    struct EnumContext {
        HMONITOR* entries;
        int capacity;
        int count;
    } context{entries, capacity, 0};

    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
            auto* context = reinterpret_cast<EnumContext*>(lParam);

            if (context -> count >= context -> capacity) {
                return FALSE;
            }

            context -> entries[context -> count] = monitor;
            context -> count++;

            return TRUE;
        },
        reinterpret_cast<LPARAM>(&context)
    );

    return context.count;
}

/// Copies the cached monitor list into the caller's buffer (capacity kMaxCachedMonitors), rebuilding the cache outside the lock when it is stale
int SnapshotMonitors(HMONITOR* entries) {
    DWORD now = GetTickCount();

    AcquireSRWLockShared(&g_monitorCacheLock);

    if (g_monitorCacheValid && now - g_monitorCacheTick < kMonitorCacheTtlMs) {
        int count = g_monitorCacheCount;

        for (int i = 0; i < count; i++) {
            entries[i] = g_monitorCacheEntries[i];
        }

        ReleaseSRWLockShared(&g_monitorCacheLock);

        return count;
    }

    ReleaseSRWLockShared(&g_monitorCacheLock);

    int count = EnumerateMonitorsUncached(entries, kMaxCachedMonitors);

    AcquireSRWLockExclusive(&g_monitorCacheLock);

    for (int i = 0; i < count; i++) {
        g_monitorCacheEntries[i] = entries[i];
    }

    g_monitorCacheCount = count;
    g_monitorCacheTick = now;
    g_monitorCacheValid = true;
    ReleaseSRWLockExclusive(&g_monitorCacheLock);

    return count;
}

/// Resolves a one-based monitor number to its HMONITOR, or nullptr when out of range
HMONITOR GetMonitorByIndex(int monitorIndex) {
    if (monitorIndex <= 0 || monitorIndex > kMaxCachedMonitors) {
        return nullptr;
    }

    HMONITOR entries[kMaxCachedMonitors];
    int count = SnapshotMonitors(entries);

    return monitorIndex <= count ? entries[monitorIndex - 1] : nullptr;
}

/// Resolves an HMONITOR to its one-based monitor number, or 0 when unknown
int GetMonitorIndex(HMONITOR monitor) {
    if (!monitor) {
        return 0;
    }

    HMONITOR entries[kMaxCachedMonitors];
    int count = SnapshotMonitors(entries);

    for (int i = 0; i < count; i++) {
        if (entries[i] == monitor) {
            return i + 1;
        }
    }

    return 0;
}

/// Calls the original (unhooked) MonitorFromWindow so internal logic never sees the mod's own redirections
HMONITOR GetActualMonitorFromWindow(HWND hWnd, DWORD flags) {
    if (MonitorFromWindow_Original) {
        return MonitorFromWindow_Original(hWnd, flags);
    }

    return MonitorFromWindow(hWnd, flags);
}

/// Calls the original (unhooked) MonitorFromPoint
HMONITOR GetActualMonitorFromPoint(POINT point, DWORD flags) {
    if (MonitorFromPoint_Original) {
        return MonitorFromPoint_Original(point, flags);
    }

    return MonitorFromPoint(point, flags);
}

/// Calls the original (unhooked) MonitorFromRect
HMONITOR GetActualMonitorFromRect(LPCRECT rect, DWORD flags) {
    if (MonitorFromRect_Original) {
        return MonitorFromRect_Original(rect, flags);
    }

    return MonitorFromRect(rect, flags);
}

/// One-based monitor number of the monitor a window is on, 0 when unknown
int GetMonitorIndexForWindow(HWND hWnd) {
    HMONITOR windowMonitor = GetActualMonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    return GetMonitorIndex(windowMonitor);
}

/// Whether the mod's styling applies to this taskbar window under the current monitor-mode settings
bool ShouldApplyToTaskbar(HWND hWnd) {
    if (GetSettings().monitorMode == MonitorMode::All) {
        return true;
    }

    int monitorIndex = GetMonitorIndexForWindow(hWnd);

    return monitorIndex > 0 && GetSettings().selectedMonitors.count(monitorIndex) != 0;
}

/// True when the components setting includes the tray surfaces (promoted icons + hidden-icons chevron)
bool WantsTray() {
    return GetSettings().components == Components::All || GetSettings().components == Components::Tray;
}

/// True when the components setting includes the control-center button
bool WantsControlCenter() {
    return GetSettings().components == Components::All || GetSettings().components == Components::ControlCenter;
}

/// Always true : Windows already shows the notification/date-time button on every taskbar, the mod only guarantees it stays visible
bool WantsNotificationCenter() {
    // On Windows 11 this is also the date/time surface, which Windows already exposes on secondary taskbars. Keep it available even in tray-only mode.
    return true;
}

/// In selected mode, the first configured monitor number : the preferred owner of Windows' singleton real tray surface. Null in all-monitors mode.
HMONITOR GetSingleSelectedMonitorForPrimaryTray() {
    if (GetSettings().monitorMode != MonitorMode::Selected ||
        GetSettings().selectedMonitorOrder.empty()) {

            return nullptr;
    }

    int monitorIndex = GetSettings().selectedMonitorOrder.front();
    HMONITOR monitor = GetMonitorByIndex(monitorIndex);

    if (!monitor) {
        Wh_Log_safe(L"taskbar-multi-tray : selected monitor %d not found", monitorIndex);
    }

    return monitor;
}

/// Monitor the real (singleton) tray surface should live on right now : the pre-unload monitor while restoring, otherwise the first selected monitor (null in all-monitors mode, where the surface is never moved)
HMONITOR GetRealPrimaryTrayTargetMonitor() {
    if (g_restoringNativeTaskbars) {
        return g_nativePrimaryRestoreMonitor;
    }

    return GetSingleSelectedMonitorForPrimaryTray();
}

HMONITOR GetActiveProxyFlyoutMonitor();
HMONITOR GetActiveFlyoutTargetMonitor();
void ClearProxyFlyoutMonitorState();

/// Returns the monitor of the currently armed flyout context, or null when none. Merges the process-local copy with the cross-process shared snapshot (adopting newer generations, honoring shortened deadlines) and lazily expires the context once its tick deadline passes. On success the process-local g_proxyFlyout* fields are synced, so callers may read them directly instead of re-running this resolution per field.
HMONITOR GetActiveProxyFlyoutMonitor() {
    // Fast path for the hot hooks : every MonitorFrom*/placement/dispatch call in two processes funnels through here, and with no process-local context and no shared context two plain reads settle it without the full seqlock snapshot. Arming publishes the monitor through the seqlock with barriers, so a racy null read here can only delay redirection by one query.
    if (!g_proxyFlyoutMonitor && !*reinterpret_cast<HMONITOR volatile*>(&g_sharedProxyState.proxyFlyoutMonitor)) {
        return nullptr;
    }

    SharedProxyState snapshot = {};
    bool hasSharedSnapshot = ReadSharedProxyState(&snapshot);
    HMONITOR proxyMonitor = g_proxyFlyoutMonitor;
    DWORD proxyUntilTick = g_proxyFlyoutUntilTick;

    if (g_proxyFlyoutMonitor && hasSharedSnapshot && g_proxyFlyoutGeneration == snapshot.proxyFlyoutGeneration) {
        if (!snapshot.proxyFlyoutMonitor || !snapshot.proxyFlyoutUntilTick) {
            // Explorer cleared the shared context. Drop the process-local copy too, otherwise ShellHost can keep redirecting after unload or after the next monitor click starts publishing a new context.
            proxyUntilTick = 0;
        } else if (static_cast<LONG>(snapshot.proxyFlyoutUntilTick - proxyUntilTick) < 0) {
            proxyUntilTick = snapshot.proxyFlyoutUntilTick;
            g_proxyFlyoutUntilTick = proxyUntilTick;
        }
    } else if (g_proxyFlyoutMonitor && hasSharedSnapshot && g_proxyFlyoutGeneration != snapshot.proxyFlyoutGeneration) {
        if (!snapshot.proxyFlyoutMonitor || !snapshot.proxyFlyoutUntilTick) {
            proxyUntilTick = 0;
        } else {
            CopySharedProxySnapshotToLocal(snapshot);
            proxyMonitor = g_proxyFlyoutMonitor;
            proxyUntilTick = g_proxyFlyoutUntilTick;
        }
    } else if (!g_proxyFlyoutMonitor && hasSharedSnapshot) {
        proxyMonitor = snapshot.proxyFlyoutMonitor;
        proxyUntilTick = snapshot.proxyFlyoutUntilTick;
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

        // Clear the shared context only when it is still the generation this read observed. An expiry racing a concurrent click on another monitor (or in the other process) must never wipe the freshly armed context, and a failed snapshot read means a writer is mid-arm right now, so leave the shared state to the next successful read.
        if (hasSharedSnapshot &&
            *reinterpret_cast<volatile LONG*>(&g_sharedProxyState.proxyFlyoutGeneration) == snapshot.proxyFlyoutGeneration) {

            ClearProxyFlyoutMonitorState();
        }

        return nullptr;
    }

    // Hot-plug protection (the PowerToys Command Palette pattern) : a context armed before a display change can hold a dead HMONITOR, and forcing monitor queries to a dead handle breaks the shell's placement math. Validate against the live topology and drop the context, the cursor rescue re-resolves a following open.
    MONITORINFO liveMonitorInfo = {};
    liveMonitorInfo.cbSize = sizeof(liveMonitorInfo);

    if (!GetMonitorInfoW(proxyMonitor, &liveMonitorInfo)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : dropping flyout monitor context, "
            L"monitor handle 0x%p died after a display change",
            proxyMonitor
        );

        InvalidateMonitorCache();

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

        // Same generation guard as the expiry path : never wipe a context another click armed concurrently
        if (hasSharedSnapshot &&
            *reinterpret_cast<volatile LONG*>(&g_sharedProxyState.proxyFlyoutGeneration) == snapshot.proxyFlyoutGeneration) {

            ClearProxyFlyoutMonitorState();
        }

        return nullptr;
    }

    if (!g_proxyFlyoutMonitor && hasSharedSnapshot) {
        CopySharedProxySnapshotToLocal(snapshot);
    }

    return proxyMonitor;
}

// Use the monitor captured when the native flyout context is armed. The previous live-cursor targeting fixed stale contexts in one case, but it can also move a delayed popup to whichever monitor the cursor reaches before placement completes. Stale contexts are cleared on later taskbar clicks, the active placement target itself must stay stable. Opens that never armed a context are handled separately by the placement-time cursor rescue (TryArmCursorRescueFlyoutContext), and contexts whose monitor died after a display change are dropped by GetActiveProxyFlyoutMonitor.
HMONITOR GetActiveFlyoutTargetMonitor() {
    return GetActiveProxyFlyoutMonitor();
}

/// True for the two flyout kinds the mod arms contexts for
bool IsKnownFlyoutKind(int kind) {
    return kind == kNativeControlCenter || kind == kNativeHiddenTray;
}

/// Rewrites the active context's expiry deadline to at most durationMs from now (never extends it) and republishes through the seqlock, so a placed popup stops influencing monitor queries quickly
void ShortenActiveFlyoutMonitorContext(DWORD durationMs, PCWSTR reason) {
    if (!GetActiveProxyFlyoutMonitor()) {
        return;
    }

    DWORD newUntilTick = GetTickCount() + durationMs;
    SharedProxyState snapshot = {};

    if (!ReadSharedProxyState(&snapshot)) {
        return;
    }

    // The rewrite below republishes this process's local context data, which is only valid while the shared generation still matches it. A mismatch means another click already armed a newer context, and shortening must never stomp it with stale data or cut its fresh deadline.
    if (snapshot.proxyFlyoutGeneration != g_proxyFlyoutGeneration) {
        return;
    }

    DWORD oldUntilTick = snapshot.proxyFlyoutUntilTick;

    if (oldUntilTick &&
        static_cast<LONG>(oldUntilTick - newUntilTick) <= 0) {

        return;
    }

    if (g_proxyFlyoutMonitor) {
        g_proxyFlyoutUntilTick = newUntilTick;
    }

    BeginSharedProxyStateWrite();
    g_sharedProxyState.proxyFlyoutMonitor = g_proxyFlyoutMonitor;
    g_sharedProxyState.proxyFlyoutMonitorIndex = g_proxyFlyoutMonitorIndex;
    g_sharedProxyState.proxyFlyoutUntilTick = newUntilTick;
    g_sharedProxyState.proxyFlyoutKind = g_proxyFlyoutKind;
    g_sharedProxyState.proxyFlyoutTaskbarWnd = reinterpret_cast<LONG_PTR>(g_proxyFlyoutTaskbarWnd);
    g_sharedProxyState.proxyFlyoutAnchorValid = g_proxyFlyoutAnchorValid ? 1 : 0;
    g_sharedProxyState.proxyFlyoutAnchorX = g_proxyFlyoutAnchorPoint.x;
    g_sharedProxyState.proxyFlyoutAnchorY = g_proxyFlyoutAnchorPoint.y;
    g_sharedProxyState.proxyFlyoutTaskbarRectValid = g_proxyFlyoutTaskbarRectValid ? 1 : 0;
    g_sharedProxyState.proxyFlyoutTaskbarLeft = g_proxyFlyoutTaskbarRect.left;
    g_sharedProxyState.proxyFlyoutTaskbarTop = g_proxyFlyoutTaskbarRect.top;
    g_sharedProxyState.proxyFlyoutTaskbarRight = g_proxyFlyoutTaskbarRect.right;
    g_sharedProxyState.proxyFlyoutTaskbarBottom = g_proxyFlyoutTaskbarRect.bottom;
    g_sharedProxyState.proxyFlyoutTargetDpi = g_proxyFlyoutTargetDpi;
    g_proxyFlyoutGeneration = EndSharedProxyStateWrite();

    // No taskbar-window timer here. Context expiry is tick-based and enforced lazily by GetActiveProxyFlyoutMonitor, so no timers have to run on Explorer's taskbar windows from arbitrary calling threads.

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : shortening active flyout monitor "
            L"context after %s until tick=%lu generation=%ld",
            reason, newUntilTick, g_proxyFlyoutGeneration
        );
    }
}

/// Per-window DPI with a 96 fallback, resolved dynamically so older systems without GetDpiForWindow keep working
UINT GetWindowDpiOrDefault(HWND hWnd) {
    using GetDpiForWindow_t = UINT(WINAPI*)(HWND);
    static GetDpiForWindow_t getDpiForWindow = reinterpret_cast<GetDpiForWindow_t>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));

    UINT dpi = hWnd && getDpiForWindow ? getDpiForWindow(hWnd) : 0;

    return dpi ? dpi : USER_DEFAULT_SCREEN_DPI;
}

/// Per-monitor effective DPI via shcore's GetDpiForMonitor, 0 when unavailable
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

/// Target-side filter for MonitorFromPoint forcing : only ambiguous near-origin points or points already on the clicked monitor are redirected, so unrelated windows keep their real monitor
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

/// Target-side filter for MonitorFromRect forcing : only ambiguous (empty/origin) rectangles or geometry already on the clicked monitor are redirected.
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

    // Force only ambiguous rectangles or rectangles already on the clicked monitor
    return actualMonitor == targetMonitor;
}

/// Center of a monitor's work area in screen coordinates
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

/// twinui.pcshell hook : while a flyout context is active, connects the immersive flyout helper to the clicked monitor's center instead of the original point, steering ShellHost-hosted flyout placement
bool WINAPI ImmersiveMonitorHelper_ConnectToMonitor_Hook(void* pThis, HWND hWnd, POINT point) {
    HMONITOR monitor = IsActiveFlyoutRedirectionSuppressed()
        ? nullptr
        : GetActiveFlyoutTargetMonitor();

    if (!monitor) {
        return ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, hWnd, point);
    }

    POINT targetPoint = {};

    if (!GetMonitorCenterPoint(monitor, &targetPoint)) {
        return ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, hWnd, point);
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : redirecting ConnectToMonitor from "
            L"(%ld,%ld) to monitor %d at (%ld,%ld)",
            point.x, point.y, GetMonitorIndex(monitor), targetPoint.x,
            targetPoint.y
        );
    }

    return ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, hWnd, targetPoint);
}

/// twinui.pcshell hook : while a flyout context is active, the periodic monitor-connection adjustment is redirected to the clicked monitor's center instead of whatever monitor ShellHost last used, falling back to the original behavior when the connect fails
HRESULT WINAPI
ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Hook(void* pThis) {
    auto original = [=]() {
        return ImmersiveMonitorHelper_AdjustMonitorConnectedIfNeeded_Original(pThis);
    };

    HMONITOR monitor = IsActiveFlyoutRedirectionSuppressed()
        ? nullptr
        : GetActiveFlyoutTargetMonitor();

    if (!monitor) {
        return original();
    }

    POINT point = {};

    if (!GetMonitorCenterPoint(monitor, &point)) {
        return original();
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : forcing immersive flyout to monitor %d "
            L"at (%ld,%ld)",
            GetMonitorIndex(monitor), point.x, point.y
        );
    }

    if (!ImmersiveMonitorHelper_ConnectToMonitor_Original(pThis, nullptr, point)) {
        Wh_Log_safe(L"taskbar-multi-tray : ConnectToMonitor failed for proxy flyout");

        return original();
    }

    return S_OK;
}

/// user32 hook : while a flyout context is active, point-to-monitor queries that pass the target-side filter resolve to the clicked monitor
HMONITOR WINAPI MonitorFromPoint_Hook(POINT pt, DWORD flags) {
    if (!IsActiveFlyoutRedirectionSuppressed()) {
        if (HMONITOR monitor = GetActiveFlyoutTargetMonitor()) {
            if (!ShouldForceActiveFlyoutForPoint(pt, monitor)) {
                HMONITOR actualMonitor = GetActualMonitorFromPoint(pt, flags);

                if (GetSettings().enableVerboseLogging) {
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

            if (GetSettings().enableVerboseLogging) {
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

    return MonitorFromPoint_Original(pt, flags);
}

/// user32 hook : while a flyout context is active, rect-to-monitor queries that pass the target-side filter resolve to the clicked monitor
HMONITOR WINAPI MonitorFromRect_Hook(LPCRECT rect, DWORD flags) {
    if (!IsActiveFlyoutRedirectionSuppressed()) {
        if (HMONITOR monitor = GetActiveFlyoutTargetMonitor()) {
            if (!ShouldForceActiveFlyoutForRect(rect, monitor)) {
                HMONITOR actualMonitor = GetActualMonitorFromRect(rect, flags);

                if (GetSettings().enableVerboseLogging) {
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

            if (GetSettings().enableVerboseLogging) {
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

    return MonitorFromRect_Original(rect, flags);
}

/// user32 hook : while a flyout context is active, window-to-monitor queries resolve to the clicked monitor, but only for the clicked taskbar itself and known flyout popup windows. Control-center popup placement also shortens the context so it cannot linger.
HMONITOR WINAPI MonitorFromWindow_Hook(HWND hWnd, DWORD flags) {
    if (!IsActiveFlyoutRedirectionSuppressed()) {
        if (HMONITOR monitor = GetActiveFlyoutTargetMonitor()) {
            // The successful resolution above synced the process-local context fields, read them directly (and the class name into a stack buffer) instead of re-running the seqlock read and heap-allocating per query
            wchar_t className[64] = {};

            if (hWnd) {
                GetClassNameW(hWnd, className, ARRAYSIZE(className));
            }

            bool nativeControlCenterOrContextMenu = g_proxyFlyoutKind == kNativeControlCenter;
            bool isNativeFlyoutPopup = wcscmp(className, L"TopLevelWindowForOverflowXamlIsland") == 0
                || wcscmp(className, L"ControlCenterWindow") == 0
                || (wcscmp(className, L"Xaml_WindowedPopupClass") == 0 && nativeControlCenterOrContextMenu);
            HWND activeTaskbarWnd = g_proxyFlyoutTaskbarWnd;
            bool isClickedTaskbar = activeTaskbarWnd && hWnd == activeTaskbarWnd;

            if (!isNativeFlyoutPopup && !isClickedTaskbar) {
                HMONITOR actualMonitor = MonitorFromWindow_Original(hWnd, flags);

                if (GetSettings().enableVerboseLogging) {
                    Wh_Log_safe(
                        L"taskbar-multi-tray : %s not forcing "
                        L"MonitorFromWindow hwnd=0x%p class=%s because it "
                        L"is not the clicked taskbar or a known flyout "
                        L"popup actual=%d active=%d clicked=0x%p",
                        TargetProcessToString(g_targetProcess), hWnd,
                        className[0] ? className : L"<null>",
                        GetMonitorIndex(actualMonitor),
                        GetMonitorIndex(monitor), activeTaskbarWnd
                    );
                }

                return actualMonitor;
            }

            if (GetSettings().enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : %s forcing MonitorFromWindow "
                    L"hwnd=0x%p class=%s to active flyout monitor %d "
                    L"clicked=0x%p",
                    TargetProcessToString(g_targetProcess), hWnd,
                    className[0] ? className : L"<null>",
                    GetMonitorIndex(monitor), activeTaskbarWnd
                );
            }

            if (wcscmp(className, L"ControlCenterWindow") == 0) {
                ShortenActiveFlyoutMonitorContext(kNativeFlyoutMonitorContextAfterPlacementMs, L"ControlCenterWindow placement");
            } else if (wcscmp(className, L"Xaml_WindowedPopupClass") == 0 && nativeControlCenterOrContextMenu) {
                ShortenActiveFlyoutMonitorContext(kNativeFlyoutMonitorContextAfterPlacementMs, L"control center context popup placement");
            }

            return monitor;
        }
    }

    return MonitorFromWindow_Original(hWnd, flags);
}

/// True for the popup window classes whose placement the mod translates (overflow island, control-center window, and the control-center windowed popup while that kind is active). Callers resolve the active context first, so the kind check reads the synced local field directly. The optional classNameResult receives the window class for further branching/logging without a heap allocation.
bool IsNativeFlyoutPopupWindow(HWND hWnd, wchar_t* classNameResult, int classNameResultCount) {
    wchar_t className[64] = {};

    if (hWnd) {
        GetClassNameW(hWnd, className, ARRAYSIZE(className));
    }

    if (classNameResult && classNameResultCount > 0) {
        lstrcpynW(classNameResult, className, classNameResultCount);
    }

    bool nativeControlCenterOrContextMenu = g_proxyFlyoutKind == kNativeControlCenter;

    return wcscmp(className, L"TopLevelWindowForOverflowXamlIsland") == 0
        || wcscmp(className, L"ControlCenterWindow") == 0
        || (wcscmp(className, L"Xaml_WindowedPopupClass") == 0 && nativeControlCenterOrContextMenu);
}

/// Work-area rectangle of a monitor
bool GetMonitorWorkRect(HMONITOR monitor, RECT* workRect) {
    MONITORINFO monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return false;
    }

    *workRect = monitorInfo.rcWork;

    return true;
}

/// Clamps value into [minValue, maxValue]
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

bool IsTaskbarWindow(HWND hWnd);
bool SetFlyoutMonitorContext(HWND taskbarWnd, DWORD durationMs, PCWSTR reason, WPARAM flyoutKind, const POINT* anchorPoint);

/// Cheap pre-filter for the cursor-located rescue path : only a known flyout popup that is not visible yet (a fresh open, never a re-placement of an already-shown flyout) and whose surface the mod manages can be rescued
bool IsCursorRescueCandidateWindow(HWND hWnd, WPARAM* flyoutKind) {
    if (!hWnd || IsWindowVisible(hWnd)) {
        return false;
    }

    wchar_t className[64] = {};

    if (!GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
        return false;
    }

    if (wcscmp(className, L"TopLevelWindowForOverflowXamlIsland") == 0) {
        if (!WantsTray()) {
            return false;
        }

        if (flyoutKind) {
            *flyoutKind = kNativeHiddenTray;
        }

        return true;
    }

    if (wcscmp(className, L"ControlCenterWindow") == 0) {
        if (!WantsControlCenter()) {
            return false;
        }

        if (flyoutKind) {
            *flyoutKind = kNativeControlCenter;
        }

        return true;
    }

    return false;
}

/// Last-chance monitor fix for flyout opens that arrive with no armed context : the click hit-test depends on the promoted-icon width measured during the last apply pass (tray-icon changes shift the chevron region), and pen/touch input bypasses the subclass left-click path entirely. The destination is resolved from the live cursor, like the start-menu-open-location mod, but scoped to fresh (not yet visible) overflow/control-center popups while the cursor sits on a managed taskbar whose monitor differs from the requested placement, so delayed re-placements can never chase the cursor and keyboard opens away from the taskbars stay native.
/// @return true when a short cursor-derived context was armed and the placement should be translated
bool TryArmCursorRescueFlyoutContext(HWND hWnd, const RECT& requestedRect) {
    WPARAM flyoutKind = 0;

    if (!IsCursorRescueCandidateWindow(hWnd, &flyoutKind)) {
        return false;
    }

    POINT cursorPoint = {};

    if (!GetCursorPos(&cursorPoint)) {
        return false;
    }

    HWND cursorWnd = WindowFromPoint(cursorPoint);
    HWND cursorTaskbarWnd = cursorWnd ? GetAncestor(cursorWnd, GA_ROOT) : nullptr;

    if (!cursorTaskbarWnd || !IsTaskbarWindow(cursorTaskbarWnd) || !ShouldApplyToTaskbar(cursorTaskbarWnd)) {
        return false;
    }

    HMONITOR cursorMonitor = GetActualMonitorFromPoint(cursorPoint, MONITOR_DEFAULTTONEAREST);
    HMONITOR requestedMonitor = GetActualMonitorFromRect(&requestedRect, MONITOR_DEFAULTTONEAREST);

    if (!cursorMonitor || cursorMonitor == requestedMonitor) {
        return false;
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : cursor rescue for unredirected flyout "
            L"open kind=%d cursor monitor=%d requested monitor=%d "
            L"taskbar=0x%p",
            static_cast<int>(flyoutKind), GetMonitorIndex(cursorMonitor),
            GetMonitorIndex(requestedMonitor), cursorTaskbarWnd
        );
    }

    return SetFlyoutMonitorContext(
        cursorTaskbarWnd, kNativeFlyoutMonitorContextAfterPlacementMs,
        L"cursor rescue", flyoutKind,
        flyoutKind == kNativeHiddenTray ? &cursorPoint : nullptr
    );
}

/// Translates a flyout popup rectangle onto the active context's monitor : rescales for source/target DPI, anchors hidden-tray popups to the captured chevron point, pins popups to the armed taskbar's edge with a small gap, preserves relative offsets otherwise, and clamps into the target work area.
/// @return true when translatedRect differs from the request and should be applied instead
bool TranslateNativeFlyoutRectToActiveMonitor(HWND hWnd, const RECT& requestedRect, RECT* translatedRect, PCWSTR apiName) {
    if (IsActiveFlyoutRedirectionSuppressed()) {
        return false;
    }

    HMONITOR targetMonitor = GetActiveFlyoutTargetMonitor();

    if (!targetMonitor) {
        // No armed context : a click the hit-test missed, pen/touch input, or a context dropped after a display change. Try the cursor rescue before letting the open land on the wrong monitor.
        if (!TryArmCursorRescueFlyoutContext(hWnd, requestedRect)) {
            return false;
        }

        targetMonitor = GetActiveFlyoutTargetMonitor();

        if (!targetMonitor) {
            return false;
        }
    }

    // The successful resolution above synced the process-local g_proxyFlyout* fields, read them directly below instead of re-running the seqlock read + liveness validation per accessor
    wchar_t className[64] = {};

    if (!IsNativeFlyoutPopupWindow(hWnd, className, ARRAYSIZE(className))) {
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
    // Derive target DPI from the active monitor and only fall back to the snapshot value captured at click time
    UINT targetDpi = GetMonitorDpiOrDefault(targetMonitor);

    if (!targetDpi) {
        targetDpi = g_proxyFlyoutTargetDpi;
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
    int activeFlyoutKind = g_proxyFlyoutKind;
    bool usedAnchor = ((wcscmp(className, L"TopLevelWindowForOverflowXamlIsland") == 0 && activeFlyoutKind == kNativeHiddenTray) || (wcscmp(className, L"Xaml_WindowedPopupClass") == 0 && activeFlyoutKind == kNativeControlCenter))
        && g_proxyFlyoutAnchorValid;

    if (usedAnchor) {
        anchorPoint = g_proxyFlyoutAnchorPoint;
        translatedLeft = anchorPoint.x - translatedWidth / 2;
        translatedTop = targetWork.bottom - translatedHeight - flyoutTaskbarGap;

        if (g_proxyFlyoutTaskbarRectValid) {
            taskbarRect = g_proxyFlyoutTaskbarRect;
        }

        if (g_proxyFlyoutTaskbarRectValid &&
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

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : moving %s via %s from "
            L"(%ld,%ld,%ld,%ld) to (%ld,%ld,%ld,%ld) on monitor %d "
            L"kind=%d anchor=%d (%ld,%ld) taskbar=(%ld,%ld,%ld,%ld) "
            L"dpi=%u -> %u",
            className, apiName, requestedRect.left,
            requestedRect.top, requestedRect.right, requestedRect.bottom,
            translatedRect -> left, translatedRect -> top,
            translatedRect -> right, translatedRect -> bottom,
            GetMonitorIndex(targetMonitor), activeFlyoutKind,
            usedAnchor, anchorPoint.x, anchorPoint.y, taskbarRect.left,
            taskbarRect.top, taskbarRect.right, taskbarRect.bottom,
            sourceDpi, targetDpi
        );
    }

    ShortenActiveFlyoutMonitorContext(kNativeFlyoutMonitorContextAfterPlacementMs, apiName);
    return true;
}

/// Cheap pre-filter for the placement hooks : only window-position changes that actually move or size are worth translating (pure z-order/activation changes are not)
bool ShouldTranslateNativeFlyoutWindowPosition(UINT uFlags) {
    if (uFlags & SWP_HIDEWINDOW) {
        return false;
    }

    if ((uFlags & SWP_NOMOVE) && (uFlags & SWP_NOSIZE)) {
        return false;
    }

    return true;
}

/// user32 hook : rewrites known flyout popup placement onto the active context's monitor
BOOL WINAPI SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
    if (IsModUnloading()) {
        return SetWindowPos_Original(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
    }

    if (!ShouldTranslateNativeFlyoutWindowPosition(uFlags) ||
        IsActiveFlyoutRedirectionSuppressed()) {

        return SetWindowPos_Original(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
    }

    // With an armed context every known popup placement is translated. With none, only a cursor-rescue candidate (a fresh, not yet visible open) is worth the GetWindowRect below : the translation then arms the rescue context or bails.
    if (GetActiveProxyFlyoutMonitor()
            ? !IsNativeFlyoutPopupWindow(hWnd, nullptr, 0)
            : !IsCursorRescueCandidateWindow(hWnd, nullptr)) {

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
    if (TranslateNativeFlyoutRectToActiveMonitor(hWnd, requestedRect, &translatedRect, L"SetWindowPos")) {
        uFlags &= ~SWP_NOMOVE;
        uFlags &= ~SWP_NOSIZE;
        X = translatedRect.left;
        Y = translatedRect.top;
        cx = translatedRect.right - translatedRect.left;
        cy = translatedRect.bottom - translatedRect.top;
    }

    return SetWindowPos_Original(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

/// user32 hook : rewrites known flyout popup placement onto the active context's monitor
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

/// user32 hook : rewrites known flyout popup placement onto the active context's monitor
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

/// user32 hook : rewrites known flyout popup placement onto the active context's monitor
HDWP WINAPI DeferWindowPos_Hook(HDWP hWinPosInfo, HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags) {
    if (IsModUnloading()) {
        return DeferWindowPos_Original(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
    }

    if (!ShouldTranslateNativeFlyoutWindowPosition(uFlags) ||
        IsActiveFlyoutRedirectionSuppressed()) {

        return DeferWindowPos_Original(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
    }

    // Same two-stage gate as SetWindowPos : armed contexts translate known popups, unarmed placements only proceed for cursor-rescue candidates
    if (GetActiveProxyFlyoutMonitor()
            ? !IsNativeFlyoutPopupWindow(hWnd, nullptr, 0)
            : !IsCursorRescueCandidateWindow(hWnd, nullptr)) {

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

    if (TranslateNativeFlyoutRectToActiveMonitor( hWnd, requestedRect, &translatedRect, L"DeferWindowPos")) {
        uFlags &= ~SWP_NOMOVE;
        uFlags &= ~SWP_NOSIZE;
        x = translatedRect.left;
        y = translatedRect.top;
        cx = translatedRect.right - translatedRect.left;
        cy = translatedRect.bottom - translatedRect.top;
    }

    return DeferWindowPos_Original(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
}

/// user32 hook : while a known flyout context is active, reports the context's display as the primary device, so display-enumeration-based placement (ShellHost flyouts) follows the clicked monitor
BOOL WINAPI EnumDisplayDevicesW_Hook(LPCWSTR deviceName, DWORD deviceIndex, PDISPLAY_DEVICEW displayDevice, DWORD flags) {
    BOOL result = EnumDisplayDevicesW_Original(deviceName, deviceIndex, displayDevice, flags);

    if (!result || !displayDevice || deviceName || IsModUnloading()) {
        return result;
    }

    // Only spoof the primary display while a known tray/control-center flyout is being opened. Earlier versions also spoofed it during secondary tray initialization, which fed the primary-like host hijack removed in 1.0.7.
    HMONITOR activeMonitor = GetActiveProxyFlyoutMonitor();

    if (!activeMonitor || !IsKnownFlyoutKind(g_proxyFlyoutKind)) {
        return result;
    }

    // While redirection is suppressed (the mod is applying/restoring taskbar XAML), spoofed queries fall back to the real-tray target monitor instead of the armed flyout monitor
    HMONITOR monitor = IsActiveFlyoutRedirectionSuppressed()
        ? GetRealPrimaryTrayTargetMonitor()
        : activeMonitor;

    if (!monitor) {
        return result;
    }

    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return result;
    }

    if (wcscmp(displayDevice -> DeviceName, monitorInfo.szDevice) == 0) {
        if (GetSettings().enableVerboseLogging) {
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

/// Sends Explorer's internal display-change message (0x5B8, handled by CTray::_HandleDisplayChange) so TrayUI::_SetStuckMonitor re-runs after a settings change or restore
void NotifyTaskbarDisplayChange(HWND taskbarWnd) {
    if (!taskbarWnd) {
        return;
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : notifying taskbar display change");
    }

    // Handled by CTray::_HandleDisplayChange, this makes Explorer re-run TrayUI::_SetStuckMonitor after settings change
    SendMessageTimeout(taskbarWnd, 0x5B8, 0, 0, SMTO_ABORTIFHUNG, 2000, nullptr);
}

/// Window class name, empty on failure
std::wstring GetWindowClassName(HWND hWnd) {
    wchar_t className[64] = {};

    if (!GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
        return L"<unknown>";
    }

    return className;
}

/// Display device name (\\.\DISPLAYn) of the monitor hosting a window, for logs
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

/// Visual-tree child count, 0 on failure
int GetVisualChildCount(FrameworkElement element) {
    try {
        return Media::VisualTreeHelper::GetChildrenCount(element);
    } catch (...) {
        return -1;
    }
}

/// UpdateLayout that swallows XAML exceptions (layout can fail transiently during startup)
void UpdateLayoutBestEffort(FrameworkElement element, PCWSTR debugName) {
    try {
        element.UpdateLayout();
    } catch (...) {
        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : failed to update layout for %s", debugName);
        }
    }
}

/// Verbose log line describing a taskbar window : class, monitor number, display device, and rectangle
void LogTaskbarWindow(HWND hWnd, PCWSTR stage) {
    if (!GetSettings().enableVerboseLogging) {
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

/// Verbose log line describing a XAML element : class, name, visibility, opacity, hit-testability, size, and child count
void LogElementState(FrameworkElement element, PCWSTR label) {
    if (!GetSettings().enableVerboseLogging) {
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

/// Raw ABI pointer of a WinRT object (nullptr-safe), used for identity comparison and logging
void* InspectableAbi(winrt::Windows::Foundation::IInspectable object) {
    return object ? winrt::get_abi(object) : nullptr;
}

/// True when the object is a XAML UIElement (UI content must not be shared between islands, unlike plain view-model objects)
bool IsXamlUiElement(winrt::Windows::Foundation::IInspectable object) {
    if (!object) {
        return false;
    }

    return static_cast<bool>(object.try_as<UIElement>());
}

/// True when the cache holds anything worth sharing
bool HasAnyBinding(const CachedXamlBinding& binding) {
    return binding.dataContext || binding.itemsSource || binding.content;
}

/// Caches the element's DataContext/ItemsSource/Content when present and different from what is already cached
/// @return true when the cache changed
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

/// The binding bridge that populates copied tray surfaces. The real tray owner's element acts as the source and only gets its bindings cached, every other taskbar's matching element receives the cached DataContext/ItemsSource/Content so it renders the singleton content. UI-element content is never shared (a UIElement cannot live in two trees). The caller batches the layout update.
/// @return true when a binding was actually applied to this element
bool SharePrimaryElementBindingIfUseful(FrameworkElement element, HWND taskbarWnd, PCWSTR debugName, CachedXamlBinding& binding, bool useThisElementAsSource, bool includeItemsSource, bool includeContent) {
    if (!element) {
        return false;
    }

    try {
        auto dataContext = element.DataContext();
        auto itemsControl = element.try_as<Controls::ItemsControl>();
        winrt::Windows::Foundation::IInspectable itemsSource = itemsControl ? itemsControl.ItemsSource() : nullptr;
        auto contentControl = element.try_as<Controls::ContentControl>();
        winrt::Windows::Foundation::IInspectable content = contentControl ? contentControl.Content() : nullptr;

        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : %s binding monitor=%d width=%.1f "
                L"source=%d dataContext=0x%p itemsControl=%d "
                L"itemsSource=0x%p contentControl=%d content=0x%p "
                L"contentIsUiElement=%d savedDataContext=0x%p "
                L"savedItemsSource=0x%p savedContent=0x%p",
                debugName, GetMonitorIndexForWindow(taskbarWnd),
                element.ActualWidth(),
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
                GetSettings().enableVerboseLogging) {
                Wh_Log_safe(L"taskbar-multi-tray : cached primary %s binding", debugName);
            }

            return false;
        }

        if (!HasAnyBinding(binding)) {
            return false;
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

        if (changed && GetSettings().enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : applied primary %s binding to monitor %d", debugName, GetMonitorIndexForWindow(taskbarWnd));
            LogElementState(element, debugName);
        }

        return changed;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to share %s binding", debugName);

        return false;
    }
}

/// Bounded recursive XAML tree dump, emitted only with verbose logging plus the treeDump setting
void DumpElementTree(FrameworkElement element, const std::wstring& label, int depth, int maxDepth) {
    if (!GetSettings().enableVerboseLogging || !GetSettings().enableTreeDump || !element) {
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

/// Tree-dump entry point for one element's children
void DumpElementChildren(FrameworkElement element, PCWSTR label) {
    DumpElementTree(element, label, 0, 2);
}

/// Forces Visible on every descendant down to maxDepth (read-compare-write so steady-state passes dirty nothing) and logs each one under verbose logging. Used on the hidden-icons stack, whose inner chevron content Windows collapses on secondary taskbars.
/// @return true when any descendant property actually changed
bool ForceVisibleDescendants(FrameworkElement element, const std::wstring& label, int depth, int maxDepth) {
    if (!element || depth > maxDepth) {
        return false;
    }

    int childCount = GetVisualChildCount(element);

    if (childCount <= 0) {
        return false;
    }

    constexpr int kMaxChildrenToVisit = 16;

    int childrenToVisit = childCount < kMaxChildrenToVisit ? childCount : kMaxChildrenToVisit;
    bool changed = false;

    for (int i = 0; i < childrenToVisit; i++) {
        try {
            auto child = Media::VisualTreeHelper::GetChild(element, i) .try_as<FrameworkElement>();

            if (!child) {
                continue;
            }

            std::wstring childLabel = label + L"/" + std::to_wstring(i);

            if (GetSettings().enableVerboseLogging) {
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

            if (child.Visibility() != Visibility::Visible) {
                child.Visibility(Visibility::Visible);
                changed = true;
            }

            if (child.Opacity() != 1.0) {
                child.Opacity(1.0);
                changed = true;
            }

            if (!child.IsHitTestVisible()) {
                child.IsHitTestVisible(true);
                changed = true;
            }

            changed |= ForceVisibleDescendants(child, childLabel, depth + 1, maxDepth);
        } catch (...) {
            Wh_Log_safe(L"taskbar-multi-tray : failed to force hidden-stack child %s/%d", label.c_str(), i);
        }
    }

    return changed;
}

/// Visits the direct FrameworkElement children until the callback returns true
/// @return the matched child, or nullptr
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

/// First FrameworkElement child, or nullptr
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

/// Direct child lookup by x:Name
FrameworkElement FindChildByName(FrameworkElement element, PCWSTR name) {
    return EnumChildElements(element, [name](FrameworkElement child) {
        return child.Name() == name;
    });
}

/// Direct child lookup by runtime class name
FrameworkElement FindChildByClassName(FrameworkElement element, PCWSTR className) {
    return EnumChildElements(element, [className](FrameworkElement child) {
        return winrt::get_class_name(child) == className;
    });
}

/// Depth-first descendant lookup by runtime class name, bounded by maxDepth
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

struct TrayElementsView {
    FrameworkElement systemTrayFrame = nullptr;
    FrameworkElement systemTrayFrameGrid = nullptr;
    FrameworkElement notifyIconStack = nullptr;
    FrameworkElement notificationAreaIcons = nullptr;
    FrameworkElement controlCenterButton = nullptr;
    FrameworkElement notificationCenterButton = nullptr;
};

/// Single-pass lookup of every tray element the apply/restore passes touch : XamlRoot content -> SystemTrayFrame -> SystemTrayFrameGrid, then one walk over the grid children instead of one full child walk per element name
bool CollectTrayElements(XamlRoot xamlRoot, HWND taskbarWnd, TrayElementsView* view, PCWSTR context) {
    FrameworkElement content = xamlRoot.Content().try_as<FrameworkElement>();

    if (!content) {
        Wh_Log_safe(L"taskbar-multi-tray : XamlRoot content is not FrameworkElement (%s)", context);

        return false;
    }

    view -> systemTrayFrame = FindChildByClassName(content, L"SystemTray.SystemTrayFrame");

    if (!view -> systemTrayFrame) {
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrame not found for monitor %d (%s)", GetMonitorIndexForWindow(taskbarWnd), context);

        return false;
    }

    view -> systemTrayFrameGrid = FindChildByName(view -> systemTrayFrame, L"SystemTrayFrameGrid");

    if (!view -> systemTrayFrameGrid) {
        Wh_Log_safe(L"taskbar-multi-tray : SystemTrayFrameGrid not found (%s)", context);

        return false;
    }

    EnumChildElements(view -> systemTrayFrameGrid, [view](FrameworkElement child) {
        auto name = child.Name();

        if (name == L"NotifyIconStack") {
            view -> notifyIconStack = child;
        } else if (name == L"NotificationAreaIcons") {
            view -> notificationAreaIcons = child;
        } else if (name == L"ControlCenterButton") {
            view -> controlCenterButton = child;
        } else if (name == L"NotificationCenterButton") {
            view -> notificationCenterButton = child;
        }

        return false;
    });

    return true;
}

/// Clears an explicit Width back to NaN (auto-size) only when one is set. Unconditionally writing NaN over NaN looks like a change to the property system (NaN != NaN) and dirtied layout on every pass.
/// @return true when an explicit width was actually cleared
bool ResetExplicitWidth(FrameworkElement element) {
    double width = element.Width();

    if (width == width) {
        element.Width(std::numeric_limits<double>::quiet_NaN());

        return true;
    }

    return false;
}

/// Makes an element Visible, fully opaque, and hit-testable. Every property is read-compare-write so a steady-state apply pass dirties nothing and the caller can batch a single layout update.
/// @return true when any property actually changed (false for missing elements and errors too, callers accumulate this for the batched layout decision)
bool ForceVisible(FrameworkElement element, PCWSTR debugName, bool resetExplicitWidth = true) {
    if (!element) {
        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : %s missing, can't force visible", debugName);
        }

        return false;
    }

    LogElementState(element, debugName);

    try {
        bool changed = false;

        if (element.Visibility() != Visibility::Visible) {
            element.Visibility(Visibility::Visible);
            changed = true;
        }

        if (element.Opacity() != 1.0) {
            element.Opacity(1.0);
            changed = true;
        }

        if (!element.IsHitTestVisible()) {
            element.IsHitTestVisible(true);
            changed = true;
        }

        if (resetExplicitWidth) {
            changed |= ResetExplicitWidth(element);
        }

        if (changed && GetSettings().enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : made %s visible", debugName);
            LogElementState(element, debugName);
        }

        return changed;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to update %s", debugName);

        return false;
    }
}

/// Shows or collapses an element (collapsing also disables hit-testing so the dead surface cannot swallow clicks). Read-compare-write like ForceVisible, the caller batches the layout update.
/// @return true when any property actually changed
bool SetElementVisibility(FrameworkElement element, PCWSTR debugName, bool visible) {
    if (!element) {
        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : %s missing, can't set visibility", debugName);
        }

        return false;
    }

    LogElementState(element, debugName);

    try {
        bool changed = false;
        Visibility targetVisibility = visible ? Visibility::Visible : Visibility::Collapsed;

        if (element.Visibility() != targetVisibility) {
            element.Visibility(targetVisibility);
            changed = true;
        }

        if (element.Opacity() != 1.0) {
            element.Opacity(1.0);
            changed = true;
        }

        if (element.IsHitTestVisible() != visible) {
            element.IsHitTestVisible(visible);
            changed = true;
        }

        if (visible) {
            changed |= ResetExplicitWidth(element);
        } else {
            if (element.MinWidth() != 0.0) {
                element.MinWidth(0.0);
                changed = true;
            }

            double width = element.Width();

            if (!(width == 0.0)) {
                element.Width(0.0);
                changed = true;
            }
        }

        if (changed && GetSettings().enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : made %s %s", debugName, visible ? L"visible" : L"collapsed");
            LogElementState(element, debugName);
        }

        return changed;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to set visibility for %s", debugName);
        return false;
    }
}

/// ForceVisible plus a minimum width. With setExactWidthWhenCollapsed, an element that layout collapsed gets the width pinned exactly while collapsed and released back to auto once layout produced real content, otherwise any explicit width is released. Matches the old reset-every-pass behavior without dirtying layout on no-op passes.
/// @return true when any property actually changed
bool ForceVisibleWithMinWidth(FrameworkElement element, PCWSTR debugName, double minWidth, bool setExactWidthWhenCollapsed = false) {
    if (!element) {
        return ForceVisible(element, debugName, false);
    }

    bool changed = ForceVisible(element, debugName, false);

    try {
        if (element.MinWidth() < minWidth) {
            element.MinWidth(minWidth);
            changed = true;
        }

        double width = element.Width();
        double actualWidth = element.ActualWidth();
        bool hasExplicitWidth = width == width;

        if (setExactWidthWhenCollapsed) {
            if (actualWidth < minWidth) {
                // Layout collapsed this element, pin the width exactly so it regains room
                if (!hasExplicitWidth || width != minWidth) {
                    element.Width(minWidth);
                    changed = true;
                }
            } else if (hasExplicitWidth) {
                // Content laid out fine, release the pin so the element can auto-size again
                element.Width(std::numeric_limits<double>::quiet_NaN());
                changed = true;
            }
        } else if (hasExplicitWidth) {
            element.Width(std::numeric_limits<double>::quiet_NaN());
            changed = true;
        }

        if (changed && GetSettings().enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : forced %s minWidth=%.1f "
                L"exactWidth=%d",
                debugName, minWidth, setExactWidthWhenCollapsed
            );
            LogElementState(element, debugName);
        }

        if (GetSettings().enableVerboseLogging && element.ActualWidth() < 4.0) {
            Wh_Log_safe(
                L"taskbar-multi-tray : %s is still width-collapsed; the "
                L"XAML element exists but Windows didn't populate/layout "
                L"usable content here",
                debugName
            );
        }

        return changed;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to size %s", debugName);

        return changed;
    }
}

/// Sets MinWidth on a tray frame container so a collapsed frame regains enough room for the forced children. The caller batches the layout update.
/// @return true when any property actually changed
bool ApplyFrameMinWidth(FrameworkElement element, PCWSTR debugName, double minWidth) {
    if (!element) {
        return false;
    }

    try {
        bool changed = false;

        if (minWidth <= 0) {
            if (element.MinWidth() != 0.0) {
                element.MinWidth(0.0);
                changed = true;
            }

            changed |= ResetExplicitWidth(element);

            return changed;
        }

        if (element.MinWidth() != minWidth) {
            element.MinWidth(minWidth);
            changed = true;

            if (GetSettings().enableVerboseLogging) {
                Wh_Log_safe(L"taskbar-multi-tray : forced %s minWidth=%.1f", debugName, minWidth);
                LogElementState(element, debugName);
            }
        }

        return changed;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to size %s", debugName);

        return false;
    }
}

/// True for the primary taskbar window class (Shell_TrayWnd)
bool IsPrimaryTaskbarWindow(HWND hWnd) {
    wchar_t className[32] = {};

    return GetClassNameW(hWnd, className, ARRAYSIZE(className)) && _wcsicmp(className, L"Shell_TrayWnd") == 0;
}

/// True for either taskbar window class (Shell_TrayWnd/Shell_SecondaryTrayWnd)
bool IsTaskbarWindow(HWND hWnd) {
    wchar_t className[32] = {};

    return GetClassNameW(hWnd, className, ARRAYSIZE(className)) && (_wcsicmp(className, L"Shell_TrayWnd") == 0 || _wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0);
}

/// True when this taskbar lives on the monitor that should own the real tray surface. In all-monitors mode that is simply the primary taskbar.
bool IsRealPrimaryTrayTargetWindow(HWND hWnd) {
    HMONITOR targetMonitor = GetRealPrimaryTrayTargetMonitor();

    if (targetMonitor) {
        return MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST) == targetMonitor;
    }

    return IsPrimaryTaskbarWindow(hWnd);
}

/// True when this taskbar should act as the binding source : it is the real-tray owner and its icon area actually has content (the width gate avoids caching empty early-startup bindings)
bool IsCurrentPrimaryTrayBindingSource(HWND taskbarWnd, FrameworkElement notificationAreaIcons) {
    if (!notificationAreaIcons || notificationAreaIcons.ActualWidth() < 4.0) {
        return false;
    }

    return IsRealPrimaryTrayTargetWindow(taskbarWnd);
}

void UpdateSharedSurfaceContextGestures(FrameworkElement element, HWND taskbarWnd, PCWSTR debugName);
void EnableSharedSurfaceContextGestures(FrameworkElement element, HWND taskbarWnd, PCWSTR debugName);

// Re-attaches the button's current ItemsSource in place. The per-glyph context-menu state follows the most recent attach, so this moves the single menu ownership to this button's island without changing the rendered content.
bool ReattachControlCenterItemsSource(FrameworkElement controlCenterButton, PCWSTR reason) {
    try {
        auto itemsControl = controlCenterButton ? controlCenterButton.try_as<Controls::ItemsControl>() : nullptr;
        winrt::Windows::Foundation::IInspectable itemsSource = itemsControl ? itemsControl.ItemsSource() : nullptr;

        if (!itemsSource) {
            return false;
        }

        itemsControl.ItemsSource(nullptr);
        itemsControl.ItemsSource(itemsSource);
        UpdateLayoutBestEffort(controlCenterButton, reason);

        return true;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to re-attach ControlCenterButton items source (%s)", reason);

        return false;
    }
}

/// Resets a non-selected taskbar to the default secondary look : tray and control-center surfaces hidden, the clock kept, context gestures re-enabled. Property writes are batched into a single layout update.
/// @return true when the tray structure was found and processed (regardless of whether anything had to change)
bool ApplyNonTargetStyle(XamlRoot xamlRoot, HWND taskbarWnd) {
    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : resetting skipped monitor %d to "
            L"default secondary tray state",
            GetMonitorIndexForWindow(taskbarWnd)
        );
    }

    try {
        TrayElementsView view;

        if (!CollectTrayElements(xamlRoot, taskbarWnd, &view, L"non-target reset")) {
            return false;
        }

        bool changed = false;

        changed |= ApplyFrameMinWidth(view.systemTrayFrame, L"SystemTrayFrame", 78.0);
        changed |= ApplyFrameMinWidth(view.systemTrayFrameGrid, L"SystemTrayFrameGrid", 78.0);

        EnableSharedSurfaceContextGestures(view.notifyIconStack, taskbarWnd, L"NotifyIconStack");
        EnableSharedSurfaceContextGestures(view.notificationAreaIcons, taskbarWnd, L"NotificationAreaIcons");
        EnableSharedSurfaceContextGestures(view.controlCenterButton, taskbarWnd, L"ControlCenterButton");

        changed |= SetElementVisibility(view.notifyIconStack, L"NotifyIconStack", false);
        changed |= SetElementVisibility(view.notificationAreaIcons, L"NotificationAreaIcons", false);
        changed |= SetElementVisibility(view.controlCenterButton, L"ControlCenterButton", false);
        changed |= ForceVisibleWithMinWidth(view.notificationCenterButton, L"NotificationCenterButton", 78.0, true);

        if (changed) {
            UpdateLayoutBestEffort(view.systemTrayFrameGrid, L"SystemTrayFrameGrid");
        }

        return true;
    } catch (const winrt::hresult_error& e) {
        Wh_Log_safe(L"taskbar-multi-tray : non-target reset failed with XAML error 0x%08X", static_cast<unsigned int>(e.code().value));

        return false;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : non-target reset failed with unexpected exception");

        return false;
    }
}

/// Restores one taskbar to its native primary state during unload : every surface visible, forced min-widths cleared, gestures enabled, and control-center menu ownership handed back to this taskbar. Property writes are batched into a single layout update.
/// @return true when the tray structure was found and processed
bool ApplyNativePrimaryStyle(XamlRoot xamlRoot, HWND taskbarWnd) {
    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : restoring native primary tray state on "
            L"monitor %d",
            GetMonitorIndexForWindow(taskbarWnd)
        );
    }

    try {
        TrayElementsView view;

        if (!CollectTrayElements(xamlRoot, taskbarWnd, &view, L"native primary restore")) {
            return false;
        }

        bool changed = false;

        changed |= ApplyFrameMinWidth(view.systemTrayFrame, L"SystemTrayFrame", 0.0);
        changed |= ApplyFrameMinWidth(view.systemTrayFrameGrid, L"SystemTrayFrameGrid", 0.0);

        EnableSharedSurfaceContextGestures(view.notifyIconStack, taskbarWnd, L"NotifyIconStack");
        EnableSharedSurfaceContextGestures(view.notificationAreaIcons, taskbarWnd, L"NotificationAreaIcons");
        EnableSharedSurfaceContextGestures(view.controlCenterButton, taskbarWnd, L"ControlCenterButton");

        changed |= ForceVisible(view.notifyIconStack, L"NotifyIconStack");
        changed |= ForceVisible(FirstChildElement(view.notifyIconStack), L"NotifyIconStackChild");
        changed |= ForceVisible(FindDescendantByClassName(view.notifyIconStack, L"SystemTray.StackListView"), L"NotifyIconStackListView");
        changed |= ForceVisible(view.notificationAreaIcons, L"NotificationAreaIcons");
        changed |= ForceVisible(view.controlCenterButton, L"ControlCenterButton");
        changed |= ForceVisible(view.notificationCenterButton, L"NotificationCenterButton");

        if (changed) {
            UpdateLayoutBestEffort(view.systemTrayFrameGrid, L"SystemTrayFrameGrid");
        }

        // Hand the control-center context-menu ownership back to the native primary, otherwise a copied button bound later would keep it after the mod unloads and native right-clicks would keep crashing
        if (ReattachControlCenterItemsSource(view.controlCenterButton, L"restore native control center ownership")) {
            g_controlCenterItemsOwnerTaskbarWnd = taskbarWnd;
        }

        return true;
    } catch (const winrt::hresult_error& e) {
        Wh_Log_safe(L"taskbar-multi-tray : native primary restore failed with XAML error 0x%08X", static_cast<unsigned int>(e.code().value));

        return false;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : native primary restore failed with unexpected exception");

        return false;
    }
}

struct ContextGestureCounts {
    int elements = 0;
    int contextFlyouts = 0;
};

/// Sets IsRightTapEnabled/IsHoldingEnabled across a bounded subtree. Clearing ContextFlyouts is supported but unused : nulling a flyout is not reliably reversible without rebuilding the subtree.
ContextGestureCounts SetContextGesturesRecursive(DependencyObject element, bool enable, bool clearContextFlyouts, int depth, int maxDepth) {
    ContextGestureCounts counts = {};

    if (!element || depth > maxDepth) {
        return counts;
    }

    try {
        if (UIElement uiElement = element.try_as<UIElement>()) {
            if (uiElement.IsRightTapEnabled() != enable) {
                uiElement.IsRightTapEnabled(enable);
            }

            if (uiElement.IsHoldingEnabled() != enable) {
                uiElement.IsHoldingEnabled(enable);
            }

            counts.elements++;

            // Avoid clearing flyouts. Some Windows builds create these dynamically, and nulling an existing flyout is not reliably reversible without rebuilding the XAML subtree.
            if (!enable && clearContextFlyouts && uiElement.ContextFlyout()) {
                uiElement.ContextFlyout(nullptr);
                counts.contextFlyouts++;
            }
        }

        int childCount = Media::VisualTreeHelper::GetChildrenCount(element);

        for (int i = 0; i < childCount; i++) {
            ContextGestureCounts childCounts = SetContextGesturesRecursive(
                Media::VisualTreeHelper::GetChild(element, i),
                enable,
                clearContextFlyouts,
                depth + 1,
                maxDepth
            );
            counts.elements += childCounts.elements;
            counts.contextFlyouts += childCounts.contextFlyouts;
        }
    } catch (...) {
    }

    return counts;
}

/// Applies SetContextGesturesRecursive to one tray surface, with verbose logging of the touched element count
void UpdateContextGesturesForElement(FrameworkElement element, HWND taskbarWnd, PCWSTR debugName, bool enable, bool clearContextFlyouts) {
    if (!element || !IsTaskbarWindow(taskbarWnd)) {
        return;
    }

    try {
        ContextGestureCounts counts = SetContextGesturesRecursive(element, enable, clearContextFlyouts, 0, 8);

        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : %s taskbar %s context gestures "
                L"monitor=%d elements=%d contextFlyouts=%d",
                enable ? L"enabled" : L"disabled",
                debugName ? debugName : L"surface",
                GetMonitorIndexForWindow(taskbarWnd),
                counts.elements,
                counts.contextFlyouts
            );
        }
    } catch (...) {
        Wh_Log_safe(
            L"taskbar-multi-tray : failed to update %s context gestures",
            debugName ? debugName : L"surface"
        );
    }
}

/// Keeps context gestures fully native (enabled) on a shared tray surface. Tray icon right-clicks are forwarded to the owning apps through the notify-icon pipeline, so they are island-safe, unlike the control-center menus.
void UpdateSharedSurfaceContextGestures(FrameworkElement element, HWND taskbarWnd, PCWSTR debugName) {
    // Keep local native context gestures enabled on shared tray surfaces (promoted icons, hidden-icons chevron). Their right-clicks are forwarded to the owning apps through the notify-icon pipeline instead of the singleton control-center menu pipeline, and the mod never forwards, synthesizes, re-owns, or suppresses them.
    UpdateContextGesturesForElement(element, taskbarWnd, debugName, true, false);
}

/// Restore-path variant : re-enables context gestures on a surface (also undoes the gesture experiments of older mod versions after an upgrade)
void EnableSharedSurfaceContextGestures(FrameworkElement element, HWND taskbarWnd, PCWSTR debugName) {
    UpdateContextGesturesForElement(element, taskbarWnd, debugName, true, false);
}

/// The per-taskbar apply pass. Forces the configured tray/control-center surfaces visible with usable widths, shares the real tray owner's bindings into copies, tracks control-center menu ownership, keeps gestures native, and caches the icon-area width used by click hit-testing. Skipped taskbars are reset to the default secondary look instead. All element lookups happen in one child walk (CollectTrayElements), every property write is read-compare-write, and a single layout update runs at the end only when something actually changed, so steady-state passes are no-ops.
/// @return true when the tray structure was found and processed
bool ApplyStyle(XamlRoot xamlRoot, HWND taskbarWnd) {
    LogTaskbarWindow(taskbarWnd, L"ApplyStyle");

    if (!ShouldApplyToTaskbar(taskbarWnd)) {
        return ApplyNonTargetStyle(xamlRoot, taskbarWnd);
    }

    try {
        TrayElementsView view;

        if (!CollectTrayElements(xamlRoot, taskbarWnd, &view, L"apply")) {
            return false;
        }

        if (GetSettings().enableTreeDump) {
            DumpElementChildren(view.systemTrayFrameGrid, L"SystemTrayFrameGrid");
        }

        bool changed = false;

        changed |= ForceVisible(view.systemTrayFrame, L"SystemTrayFrame");
        changed |= ForceVisible(view.systemTrayFrameGrid, L"SystemTrayFrameGrid");

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

        changed |= ApplyFrameMinWidth(view.systemTrayFrame, L"SystemTrayFrame", frameMinWidth);
        changed |= ApplyFrameMinWidth(view.systemTrayFrameGrid, L"SystemTrayFrameGrid", frameMinWidth);

        bool useThisTaskbarAsPrimaryTrayBindingSource = IsRealPrimaryTrayTargetWindow(taskbarWnd);

        if (WantsTray()) {
            changed |= ForceVisibleWithMinWidth(view.notifyIconStack, L"NotifyIconStack", 32.0, false);
            FrameworkElement notifyIconStackChild = FirstChildElement(view.notifyIconStack);
            changed |= ForceVisibleWithMinWidth(notifyIconStackChild, L"NotifyIconStackChild", 32.0, false);
            FrameworkElement notifyIconStackListView = FindDescendantByClassName(view.notifyIconStack, L"SystemTray.StackListView");
            changed |= ForceVisibleWithMinWidth(notifyIconStackListView, L"NotifyIconStackListView", 32.0, false);
            changed |= ForceVisibleDescendants(view.notifyIconStack, L"NotifyIconStack", 0, 6);

            // The binding-source decision below reads ActualWidth, flush the layout first when this pass already changed geometry (free on steady-state no-op passes), otherwise a just-unhidden owner tray could be misread as empty for one pass
            if (changed) {
                UpdateLayoutBestEffort(view.systemTrayFrameGrid, L"SystemTrayFrameGrid");
            }

            useThisTaskbarAsPrimaryTrayBindingSource = IsCurrentPrimaryTrayBindingSource(taskbarWnd, view.notificationAreaIcons);
            changed |= ForceVisible(view.notificationAreaIcons, L"NotificationAreaIcons");
            changed |= SharePrimaryElementBindingIfUseful(view.notificationAreaIcons, taskbarWnd, L"NotificationAreaIcons", g_primaryNotificationAreaIconsBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, false);
            changed |= SharePrimaryElementBindingIfUseful(view.notifyIconStack, taskbarWnd, L"NotifyIconStack", g_primaryNotifyIconStackBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, true);
            changed |= SharePrimaryElementBindingIfUseful(notifyIconStackChild, taskbarWnd, L"NotifyIconStackChild", g_primaryNotifyIconStackChildBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, true);
            changed |= SharePrimaryElementBindingIfUseful(notifyIconStackListView, taskbarWnd, L"NotifyIconStackListView", g_primaryNotifyIconStackListViewBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, false);

            UpdateSharedSurfaceContextGestures(view.notificationAreaIcons, taskbarWnd, L"NotificationAreaIcons");
            UpdateSharedSurfaceContextGestures(view.notifyIconStack, taskbarWnd, L"NotifyIconStack");
        } else {
            SetCachedNotificationAreaIconsWidth(taskbarWnd, 0.0);
            changed |= SetElementVisibility(view.notifyIconStack, L"NotifyIconStack", false);
            changed |= SetElementVisibility(view.notificationAreaIcons, L"NotificationAreaIcons", false);
        }

        if (WantsControlCenter()) {
            changed |= ForceVisibleWithMinWidth(view.controlCenterButton, L"ControlCenterButton", 117.0, true);

            void* ccItemsSourceBefore = nullptr;
            void* ccItemsSourceAfter = nullptr;

            try {
                auto ccItemsControl = view.controlCenterButton ? view.controlCenterButton.try_as<Controls::ItemsControl>() : nullptr;
                ccItemsSourceBefore = ccItemsControl ? InspectableAbi(ccItemsControl.ItemsSource()) : nullptr;
            } catch (...) {
            }

            changed |= SharePrimaryElementBindingIfUseful(view.controlCenterButton, taskbarWnd, L"ControlCenterButton", g_primaryControlCenterButtonBinding, useThisTaskbarAsPrimaryTrayBindingSource, true, true);

            try {
                auto ccItemsControl = view.controlCenterButton ? view.controlCenterButton.try_as<Controls::ItemsControl>() : nullptr;
                ccItemsSourceAfter = ccItemsControl ? InspectableAbi(ccItemsControl.ItemsSource()) : nullptr;
            } catch (...) {
            }

            // Attaching the shared ItemsSource moves the single per-glyph context-menu ownership to this island, so track who attached last. Right-clicks re-attach on the clicked taskbar just in time (PrepareControlCenterContextOwnership) and the tracking keeps that re-attach skippable when the clicked taskbar already owns the menus.
            if (ccItemsSourceAfter && ccItemsSourceAfter != ccItemsSourceBefore) {
                g_controlCenterItemsOwnerTaskbarWnd = taskbarWnd;
            }

            // Context gestures stay fully native everywhere. The per-glyph menus open through a context-request path that ignores IsRightTapEnabled (proven by 1.0.7 logs : menus kept opening on the owner island with right-tap disabled), so toggling gestures neither blocks the menus nor prevents the cross-island crash. The crash is prevented by moving the menu ownership to the clicked island instead.
            UpdateSharedSurfaceContextGestures(view.controlCenterButton, taskbarWnd, L"ControlCenterButton");
        } else {
            changed |= SetElementVisibility(view.controlCenterButton, L"ControlCenterButton", false);
        }

        if (WantsNotificationCenter()) {
            changed |= ForceVisibleWithMinWidth(view.notificationCenterButton, L"NotificationCenterButton", 78.0, true);
        } else {
            changed |= SetElementVisibility(view.notificationCenterButton, L"NotificationCenterButton", false);
        }

        if (changed) {
            UpdateLayoutBestEffort(view.systemTrayFrameGrid, L"SystemTrayFrameGrid");
        }

        if (WantsTray()) {
            // Measure after the single batched layout pass so the cached width feeding the click hit-test reflects this pass, not the previous one (stale widths were the main wrong-monitor flyout cause fixed in 1.0.9)
            try {
                double notificationAreaIconsWidth = view.notificationAreaIcons ? view.notificationAreaIcons.ActualWidth() : 0.0;
                SetCachedNotificationAreaIconsWidth(taskbarWnd, notificationAreaIconsWidth);

                if (GetSettings().enableVerboseLogging) {
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
        }

        return true;
    } catch (const winrt::hresult_error& e) {
        Wh_Log_safe(L"taskbar-multi-tray : ApplyStyle failed with XAML error 0x%08X (taskbar structure drift?)", static_cast<unsigned int>(e.code().value));

        return false;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : ApplyStyle failed with unexpected exception");

        return false;
    }
}

constexpr UINT_PTR kTaskbarSubclassId = 1;
constexpr int kShowDesktopWidth = 12;
constexpr int kNotificationCenterWidth = 78;
constexpr int kControlCenterWidth = 117;
constexpr int kNotifyIconStackWidth = 32;
constexpr int kNativeFlyoutHitSlop = 8;
// Use a mod-unique timer id. Small ids like 2-5 can collide with Explorer's own Shell_TrayWnd/Shell_SecondaryTrayWnd timers : SetTimer would silently replace the native timer and the subclass would swallow and kill it.
constexpr UINT_PTR kDeferredApplySettingsTimerId = 0x54424D54; // "TBMT"

void ApplySettingsFromTaskbarThread(void*);
void BeginNativeFlyoutMonitorContext(HWND taskbarWnd, PCWSTR reason, WPARAM flyoutKind, const POINT* anchorPoint);
XamlRoot GetTaskbarXamlRoot(HWND taskbarWnd);
XamlRoot GetSecondaryTaskbarXamlRoot(HWND secondaryTaskbarWnd);

/// Scales a 96-DPI design metric to the given DPI. Callers fetch the window DPI once per pass instead of once per metric.
int ScaleTaskbarMetricForDpi(UINT dpi, int value) {
    return MulDiv(value, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI);
}

/// Scales a 96-DPI design metric (fractional) to the given DPI
int ScaleTaskbarMetricForDpi(UINT dpi, double value) {
    return MulDiv(static_cast<int>(value + 0.5), static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI);
}

/// True for the secondary taskbar window class (Shell_SecondaryTrayWnd)
bool IsSecondaryTaskbarWindow(HWND hWnd) {
    wchar_t className[32] = {};

    return GetClassNameW(hWnd, className, ARRAYSIZE(className)) && _wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0;
}

/// Maps a click on a copied secondary-taskbar surface to what was hit (control center/hidden tray/nothing), using DPI-scaled fixed widths measured from the right edge : [show desktop][notification+clock][control center][promoted icons][chevron]. The promoted-icon width comes from the cached ApplyStyle measurement, the layout assumption is a horizontal bottom taskbar, vertical taskbars bail out.
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

    UINT dpi = GetWindowDpiOrDefault(hWnd);
    int xFromRight = clientRect.right - x;
    int hitSlop = ScaleTaskbarMetricForDpi(dpi, kNativeFlyoutHitSlop);
    int notificationStart = ScaleTaskbarMetricForDpi(dpi, kShowDesktopWidth);
    int notificationEnd = notificationStart + ScaleTaskbarMetricForDpi(dpi, kNotificationCenterWidth);
    int controlStart = notificationEnd;
    int controlEnd = controlStart + ScaleTaskbarMetricForDpi(dpi, kControlCenterWidth);
    int notificationAreaIconsWidth = ScaleTaskbarMetricForDpi(dpi, GetCachedNotificationAreaIconsWidth(hWnd));
    int hiddenTrayStart = controlEnd + notificationAreaIconsWidth;
    int hiddenTrayEnd = hiddenTrayStart + ScaleTaskbarMetricForDpi(dpi, kNotifyIconStackWidth);
    WPARAM result = 0;
    bool inNotificationCenterRange = false;

    if (WantsNotificationCenter() && xFromRight >= notificationStart && xFromRight < notificationEnd) {
        // Leave the notification/date-time surface fully native. Windows already opens this flyout on the clicked monitor, and the removed Win+N proxy used to move the real tray around, refreshing every taskbar (most visibly on mixed-DPI monitors).
        inNotificationCenterRange = true;
    } else if (WantsControlCenter() && xFromRight >= controlStart - hitSlop && xFromRight < controlEnd + hitSlop) {
        result = kNativeControlCenter;
    } else if (WantsTray() && xFromRight >= hiddenTrayStart - hitSlop && xFromRight < hiddenTrayEnd + hitSlop) {
        result = kNativeHiddenTray;
    }

    if (GetSettings().enableVerboseLogging && xFromRight >= notificationStart - hitSlop && xFromRight < hiddenTrayEnd + ScaleTaskbarMetricForDpi(dpi, 32)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : secondary click monitor=%d x=%d y=%d "
            L"xFromRight=%d iconWidth=%d hit=%s",
            GetMonitorIndexForWindow(hWnd), x, y, xFromRight,
            notificationAreaIconsWidth,
            result == kNativeHiddenTray
                ? L"hidden tray native"
                : result == kNativeControlCenter
                    ? L"control center native"
                    : inNotificationCenterRange
                        ? L"notification center native"
                        : L"none"
        );
    }

    return result;
}

/// True when a taskbar client point lies in the control-center strip (same right-edge metrics as HitTestProxyFlyout, valid for any styled taskbar including the primary). Gate order is cheapest-first because the hover pre-arm calls this on cursor movement.
bool IsControlCenterClientPoint(HWND hWnd, POINT point) {
    if (!WantsControlCenter() || !IsTaskbarWindow(hWnd) || !ShouldApplyToTaskbar(hWnd)) {
        return false;
    }

    RECT clientRect = {};

    if (!GetClientRect(hWnd, &clientRect)) {
        return false;
    }

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    if (width <= height || point.x < clientRect.left || point.x >= clientRect.right || point.y < clientRect.top || point.y >= clientRect.bottom) {
        return false;
    }

    UINT dpi = GetWindowDpiOrDefault(hWnd);
    int xFromRight = clientRect.right - point.x;
    int hitSlop = ScaleTaskbarMetricForDpi(dpi, kNativeFlyoutHitSlop);
    int controlStart = ScaleTaskbarMetricForDpi(dpi, kShowDesktopWidth) + ScaleTaskbarMetricForDpi(dpi, kNotificationCenterWidth);
    int controlEnd = controlStart + ScaleTaskbarMetricForDpi(dpi, kControlCenterWidth);

    return xFromRight >= controlStart - hitSlop && xFromRight < controlEnd + hitSlop;
}

/// Extracts a taskbar-client-space point from a right-click/context message. Screen-coordinate messages (WM_CONTEXTMENU, non-client) are converted, keyboard-sourced WM_CONTEXTMENU (lParam -1) yields false.
bool TryGetTaskbarContextClientPoint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, POINT* point) {
    if (!point) {
        return false;
    }

    if (uMsg == WM_CONTEXTMENU || uMsg == WM_NCRBUTTONDOWN || uMsg == WM_NCRBUTTONUP) {
        if (uMsg == WM_CONTEXTMENU && lParam == -1) {
            return false;
        }

        POINT screenPoint = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (!ScreenToClient(hWnd, &screenPoint)) {
            return false;
        }

        *point = screenPoint;
        return true;
    }

    *point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    return true;
}

FrameworkElement FindSystemTrayFrameGridForTaskbar(HWND taskbarWnd);

/// Walks up the visual tree (bounded) looking for an ancestor named ControlCenterButton. Used to scope the ShowAt hook to control-center menus only.
bool IsElementInsideControlCenterButton(DependencyObject element) {
    for (int i = 0; element && i < 16; i++) {
        try {
            if (FrameworkElement frameworkElement = element.try_as<FrameworkElement>()) {
                if (frameworkElement.Name() == L"ControlCenterButton") {
                    return true;
                }
            }

            element = Media::VisualTreeHelper::GetParent(element);
        } catch (...) {
            return false;
        }
    }

    return false;
}

// FlyoutBase::ShowAt is the exact call that kills Explorer when the cached per-glyph menu is anchored to a foreign island, and the cached instance is reachable nowhere else (not on any visual tree, and Windows.UI.Xaml exposes no Popup.AssociatedFlyout, that API is WinUI-only). The ShowAt ABI vtable slots of the flyout classes are therefore patched directly : the hook stays a pure pass-through unless the target sits inside a ControlCenterButton subtree, in which case the show is prepared (re-root or proxy) right before the native ShowAt runs.
struct FlyoutShowAtSlotHook {
    void** slot = nullptr;
    void* original = nullptr;
};

FlyoutShowAtSlotHook g_menuFlyoutShowAtSlot;
FlyoutShowAtSlotHook g_menuFlyoutShowAtWithOptionsSlot;
FlyoutShowAtSlotHook g_plainFlyoutShowAtSlot;
FlyoutShowAtSlotHook g_plainFlyoutShowAtWithOptionsSlot;

// IInspectable occupies the first 6 vtable slots. IFlyoutBase : get/put_Placement, add/remove_Opened, add/remove_Closed, add/remove_Opening, then ShowAt. IFlyoutBase5 : get/put_ShowMode, get_InputDevicePrefersPrimaryCommands, get/put_AreOpenCloseAnimationsEnabled, get_IsOpen, then ShowAt.
constexpr size_t kFlyoutBaseShowAtSlot = 14;
constexpr size_t kFlyoutBase5ShowAtSlot = 12;

using FlyoutBase_ShowAt_t = int32_t(__stdcall*)(void* pThis, void* placementTarget);
using FlyoutBase_ShowAtWithOptions_t = int32_t(__stdcall*)(void* pThis, void* placementTarget, void* showOptions);

enum class FlyoutReRootResult {
    AlreadyMatching,
    ReRooted,
    StillMismatched,
    Error,
};

// A flyout's XamlRoot is settable only while it has never been shown (logs : the first show on monitor 3 re-rooted fine, the reused instance locked to monitor 3 then threw). The full sequence Hide -> clear to null -> set is tried because clearing the association first is the only chance of unlocking a flyout that has already been shown, whichever step throws is swallowed and the final root is re-read to decide success.
FlyoutReRootResult TryReRootFlyout(Controls::Primitives::FlyoutBase const& flyout, XamlRoot const& targetRoot) {
    if (!flyout || !targetRoot) {
        return FlyoutReRootResult::Error;
    }

    try {
        XamlRoot currentRoot = nullptr;

        try {
            currentRoot = flyout.XamlRoot();
        } catch (...) {
        }

        if (currentRoot == targetRoot) {
            return FlyoutReRootResult::AlreadyMatching;
        }

        try {
            flyout.Hide();
        } catch (...) {
        }

        try {
            flyout.XamlRoot(XamlRoot{nullptr});
        } catch (...) {
        }

        try {
            flyout.XamlRoot(targetRoot);
        } catch (...) {
        }

        XamlRoot afterRoot = nullptr;

        try {
            afterRoot = flyout.XamlRoot();
        } catch (...) {
        }

        return afterRoot == targetRoot ? FlyoutReRootResult::ReRooted : FlyoutReRootResult::StillMismatched;
    } catch (...) {
        return FlyoutReRootResult::Error;
    }
}

/// Identity check against the per-island proxies, so the ShowAt hook lets the mod's own proxy shows pass straight through to the native implementation
bool IsOwnControlCenterProxyFlyout(Controls::Primitives::FlyoutBase const& flyout) {
    if (!flyout) {
        return false;
    }

    for (const auto& entry : g_controlCenterProxyFlyouts.entries) {
        try {
            if (entry.proxy && flyout == entry.proxy.try_as<Controls::Primitives::FlyoutBase>()) {
                return true;
            }
        } catch (...) {
        }
    }

    return false;
}

/// Returns the island's proxy entry, creating the proxy MenuFlyout on first use. The proxy's XamlRoot is set before its first show, which is the only moment the property is settable, and locks it to exactly the island it must serve.
ControlCenterProxyFlyoutEntry* GetOrCreateControlCenterProxy(XamlRoot const& root) {
    if (!root) {
        return nullptr;
    }

    for (auto& entry : g_controlCenterProxyFlyouts.entries) {
        if (entry.proxyRoot == root) {
            return &entry;
        }
    }

    try {
        Controls::MenuFlyout proxy;
        proxy.XamlRoot(root);

        ControlCenterProxyFlyoutEntry entry;
        entry.proxy = proxy;
        entry.proxyRoot = root;
        g_controlCenterProxyFlyouts.entries.push_back(entry);

        return &g_controlCenterProxyFlyouts.entries.back();
    } catch (...) {
        return nullptr;
    }
}

// Returns every borrowed item set from the proxies back to its original cached flyout. Run before any other control-center menu opens, so the home-island menu (shown natively) is never found empty because its items are sitting in another island's proxy.
void ReturnAllControlCenterProxyItems() {
    for (auto& entry : g_controlCenterProxyFlyouts.entries) {
        if (!entry.itemsSource || !entry.proxy) {
            entry.itemsSource = nullptr;

            continue;
        }

        try {
            auto proxyItems = entry.proxy.Items();
            auto sourceItems = entry.itemsSource.Items();
            std::vector<Controls::MenuFlyoutItemBase> moved;

            uint32_t count = proxyItems.Size();

            for (uint32_t i = 0; i < count; i++) {
                moved.push_back(proxyItems.GetAt(i));
            }

            proxyItems.Clear();

            // Only hand the items back when the home menu is still empty. If Windows rebuilt its own items in the meantime, keep ours out to avoid duplicating the whole menu.
            if (sourceItems.Size() == 0) {
                for (const auto& item : moved) {
                    sourceItems.Append(item);
                }
            }
        } catch (...) {
        }

        entry.itemsSource = nullptr;
    }
}

// Moves the locked cached menu's items into the clicked island's proxy and shows the proxy there. The items carry their native command handlers, so the per-glyph menu works on any monitor. Returns false (so the caller still suppresses the crashy native ShowAt) when the menu is not a MenuFlyout or the move fails.
bool ShowControlCenterMenuViaProxy(Controls::Primitives::FlyoutBase const& cachedFlyout, FrameworkElement const& targetElement, XamlRoot const& targetRoot) {
    auto cachedMenu = cachedFlyout.try_as<Controls::MenuFlyout>();

    if (!cachedMenu || !targetElement) {
        return false;
    }

    ControlCenterProxyFlyoutEntry* entry = GetOrCreateControlCenterProxy(targetRoot);

    if (!entry || !entry -> proxy) {
        return false;
    }

    try {
        auto proxyItems = entry -> proxy.Items();

        if (proxyItems.Size() != 0) {
            // ReturnAllControlCenterProxyItems should have emptied it. If not, refuse rather than mix two menus.
            return false;
        }

        auto sourceItems = cachedMenu.Items();
        std::vector<Controls::MenuFlyoutItemBase> moved;
        uint32_t count = sourceItems.Size();

        for (uint32_t i = 0; i < count; i++) {
            moved.push_back(sourceItems.GetAt(i));
        }

        if (moved.empty()) {
            return false;
        }

        sourceItems.Clear();

        for (const auto& item : moved) {
            proxyItems.Append(item);
        }

        entry -> itemsSource = cachedMenu;
        // Re-enters the ShowAt hook, where IsOwnControlCenterProxyFlyout short-circuits to the native show on the correct island
        entry -> proxy.ShowAt(targetElement);

        return true;
    } catch (...) {
        return false;
    }
}

// Returns true to suppress the native ShowAt. Showing a flyout whose XamlRoot does not match the placement target's island is the exact call that kills Explorer, so a locked cached menu is re-shown through the clicked island's proxy instead, and the native ShowAt is always suppressed for the locked case.
bool PrepareControlCenterFlyoutShowAt(void* flyoutAbi, void* targetAbi, PCWSTR source) {
    if (IsModUnloading() || !flyoutAbi || !targetAbi) {
        return false;
    }

    try {
        winrt::Windows::Foundation::IInspectable targetInspectable{nullptr};
        winrt::copy_from_abi(targetInspectable, targetAbi);
        DependencyObject targetElement = targetInspectable.try_as<DependencyObject>();

        if (!targetElement || !IsElementInsideControlCenterButton(targetElement)) {
            return false;
        }

        winrt::Windows::Foundation::IInspectable flyoutInspectable{nullptr};
        winrt::copy_from_abi(flyoutInspectable, flyoutAbi);
        auto flyout = flyoutInspectable.try_as<Controls::Primitives::FlyoutBase>();

        if (!flyout) {
            return false;
        }

        // Our own proxy showing itself on its island : let the native ShowAt run and never touch its borrowed items
        if (IsOwnControlCenterProxyFlyout(flyout)) {
            return false;
        }

        // Any non-proxy control-center menu is about to open : first hand every borrowed item set back to its home flyout so a home-island menu is never empty
        ReturnAllControlCenterProxyItems();

        XamlRoot targetRoot = nullptr;

        if (UIElement targetUiElement = targetElement.try_as<UIElement>()) {
            targetRoot = targetUiElement.XamlRoot();
        }

        FlyoutReRootResult result = TryReRootFlyout(flyout, targetRoot);

        // AlreadyMatching/ReRooted : the flyout can show natively on the target island
        if (result == FlyoutReRootResult::AlreadyMatching || result == FlyoutReRootResult::ReRooted) {
            if (GetSettings().enableVerboseLogging) {
                Wh_Log_safe(
                    L"taskbar-multi-tray : control center flyout %s "
                    L"result=%s suppress=0",
                    source,
                    result == FlyoutReRootResult::AlreadyMatching ? L"already-matching" : L"re-rooted"
                );
            }

            return false;
        }

        // Locked to another island : show the menu through this island's proxy instead. The native ShowAt is suppressed either way so it can never crash.
        FrameworkElement targetElementFe = targetElement.try_as<FrameworkElement>();
        bool shownViaProxy = ShowControlCenterMenuViaProxy(flyout, targetElementFe, targetRoot);

        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : control center flyout %s "
                L"result=locked viaProxy=%d suppress=1",
                source, shownViaProxy
            );
        }

        return true;
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to prepare control center flyout %s", source);
    }

    return false;
}

/// Patched IFlyoutBase::ShowAt slot of MenuFlyout. Returns S_OK without showing when the control-center preparation decided to suppress.
int32_t __stdcall MenuFlyout_ShowAt_VtableHook(void* pThis, void* placementTarget) {
    if (PrepareControlCenterFlyoutShowAt(pThis, placementTarget, L"ShowAt")) {
        return 0;
    }

    return reinterpret_cast<FlyoutBase_ShowAt_t>(g_menuFlyoutShowAtSlot.original)(pThis, placementTarget);
}

/// Patched IFlyoutBase5::ShowAt(options) slot of MenuFlyout
int32_t __stdcall MenuFlyout_ShowAtWithOptions_VtableHook(void* pThis, void* placementTarget, void* showOptions) {
    if (PrepareControlCenterFlyoutShowAt(pThis, placementTarget, L"ShowAt(options)")) {
        return 0;
    }

    return reinterpret_cast<FlyoutBase_ShowAtWithOptions_t>(g_menuFlyoutShowAtWithOptionsSlot.original)(pThis, placementTarget, showOptions);
}

/// Patched IFlyoutBase::ShowAt slot of Flyout
int32_t __stdcall Flyout_ShowAt_VtableHook(void* pThis, void* placementTarget) {
    if (PrepareControlCenterFlyoutShowAt(pThis, placementTarget, L"ShowAt")) {
        return 0;
    }

    return reinterpret_cast<FlyoutBase_ShowAt_t>(g_plainFlyoutShowAtSlot.original)(pThis, placementTarget);
}

/// Patched IFlyoutBase5::ShowAt(options) slot of Flyout
int32_t __stdcall Flyout_ShowAtWithOptions_VtableHook(void* pThis, void* placementTarget, void* showOptions) {
    if (PrepareControlCenterFlyoutShowAt(pThis, placementTarget, L"ShowAt(options)")) {
        return 0;
    }

    return reinterpret_cast<FlyoutBase_ShowAtWithOptions_t>(g_plainFlyoutShowAtWithOptionsSlot.original)(pThis, placementTarget, showOptions);
}

/// Swaps one ABI vtable slot to a hook function (VirtualProtect + interlocked exchange). Slots already patched in this batch are skipped because flyout classes can share an implementation vtable, and double-patching would chain hooks and break restore ordering.
bool PatchFlyoutVtableSlot(void* interfaceAbi, size_t slotIndex, void* hookFunction, FlyoutShowAtSlotHook* state, std::vector<void**>& patchedSlots) {
    if (!interfaceAbi || state -> slot) {
        return false;
    }

    void** vtable = *reinterpret_cast<void***>(interfaceAbi);
    void** slot = vtable + slotIndex;

    // Different flyout classes can share one implementation vtable. Patching the same slot twice would chain hooks and make unpatching order-sensitive, skip duplicates instead.
    for (void** patched : patchedSlots) {
        if (patched == slot) {
            return false;
        }
    }

    DWORD oldProtect = 0;

    if (!VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &oldProtect)) {
        return false;
    }

    state -> original = InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(slot), hookFunction);
    state -> slot = slot;
    VirtualProtect(slot, sizeof(void*), oldProtect, &oldProtect);
    patchedSlots.push_back(slot);

    return true;
}

/// Restores a patched vtable slot to its original function
void RestoreFlyoutVtableSlot(FlyoutShowAtSlotHook* state) {
    if (!state -> slot) {
        return;
    }

    DWORD oldProtect = 0;

    if (VirtualProtect(state -> slot, sizeof(void*), PAGE_READWRITE, &oldProtect)) {
        InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(state -> slot), state -> original);
        VirtualProtect(state -> slot, sizeof(void*), oldProtect, &oldProtect);
    }

    state -> slot = nullptr;
    state -> original = nullptr;
}

// Must run on the taskbar thread (XAML objects can only be created where a XAML core is initialized). Throwaway MenuFlyout/Flyout instances expose the class vtables shared by every instance of those classes, including the SystemTray-cached menus.
void EnsureFlyoutShowAtVtableHooks() {
    if (g_flyoutShowAtHooksInstalled || IsModUnloading()) {
        return;
    }

    try {
        Controls::MenuFlyout menuFlyout;
        Controls::Flyout plainFlyout;
        auto menuFlyoutBase = menuFlyout.as<Controls::Primitives::FlyoutBase>();
        auto plainFlyoutBase = plainFlyout.as<Controls::Primitives::FlyoutBase>();
        auto menuFlyoutBase5 = menuFlyout.try_as<Controls::Primitives::IFlyoutBase5>();
        auto plainFlyoutBase5 = plainFlyout.try_as<Controls::Primitives::IFlyoutBase5>();

        std::vector<void**> patchedSlots;
        int patched = 0;

        patched += PatchFlyoutVtableSlot(winrt::get_abi(menuFlyoutBase), kFlyoutBaseShowAtSlot, reinterpret_cast<void*>(MenuFlyout_ShowAt_VtableHook), &g_menuFlyoutShowAtSlot, patchedSlots);
        patched += PatchFlyoutVtableSlot(winrt::get_abi(menuFlyoutBase5), kFlyoutBase5ShowAtSlot, reinterpret_cast<void*>(MenuFlyout_ShowAtWithOptions_VtableHook), &g_menuFlyoutShowAtWithOptionsSlot, patchedSlots);
        patched += PatchFlyoutVtableSlot(winrt::get_abi(plainFlyoutBase), kFlyoutBaseShowAtSlot, reinterpret_cast<void*>(Flyout_ShowAt_VtableHook), &g_plainFlyoutShowAtSlot, patchedSlots);
        patched += PatchFlyoutVtableSlot(winrt::get_abi(plainFlyoutBase5), kFlyoutBase5ShowAtSlot, reinterpret_cast<void*>(Flyout_ShowAtWithOptions_VtableHook), &g_plainFlyoutShowAtWithOptionsSlot, patchedSlots);

        g_flyoutShowAtHooksInstalled = 1;

        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(L"taskbar-multi-tray : hooked FlyoutBase ShowAt vtable slots patched=%d", patched);
        }
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to hook FlyoutBase ShowAt vtable slots");
    }
}

/// Unload counterpart of EnsureFlyoutShowAtVtableHooks, run on the taskbar thread : returns borrowed proxy items, drops the proxies, and unpatches the ShowAt slots before the mod's code can disappear
void RemoveFlyoutShowAtVtableHooks() {
    // Return any items currently borrowed into proxies so the native cached menus are whole again, then drop the proxies, before unpatching
    ReturnAllControlCenterProxyItems();
    g_controlCenterProxyFlyouts.entries.clear();

    RestoreFlyoutVtableSlot(&g_plainFlyoutShowAtWithOptionsSlot);
    RestoreFlyoutVtableSlot(&g_plainFlyoutShowAtSlot);
    RestoreFlyoutVtableSlot(&g_menuFlyoutShowAtWithOptionsSlot);
    RestoreFlyoutVtableSlot(&g_menuFlyoutShowAtSlot);
    g_flyoutShowAtHooksInstalled = 0;
}

// The per-glyph control-center context menus (network, volume, battery) resolve their target through per-item state that follows the most recent ItemsSource attach. One shared items source feeds every taskbar's ControlCenterButton, so only the island bound last can open those menus safely : pressing the right button on any other taskbar touches that per-item state from a foreign island and Explorer dies inside native XAML, before the taskbar window receives any parent-level message. Re-attaching the items source moves the ownership to a taskbar's island, the same way the per-taskbar notification/date-time button works natively because Windows gives each island its own items.
void MoveControlCenterContextOwnershipToTaskbar(HWND taskbarWnd, PCWSTR source) {
    if (g_controlCenterItemsOwnerTaskbarWnd == taskbarWnd) {
        return;
    }

    try {
        FrameworkElement systemTrayFrameGrid = FindSystemTrayFrameGridForTaskbar(taskbarWnd);
        FrameworkElement controlCenterButton = systemTrayFrameGrid ? FindChildByName(systemTrayFrameGrid, L"ControlCenterButton") : nullptr;

        if (!ReattachControlCenterItemsSource(controlCenterButton, L"control center context ownership")) {
            return;
        }

        g_controlCenterItemsOwnerTaskbarWnd = taskbarWnd;

        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : moved control center context "
                L"ownership to monitor %d via %s",
                GetMonitorIndexForWindow(taskbarWnd), source
            );
        }
    } catch (...) {
        Wh_Log_safe(L"taskbar-multi-tray : failed to move control center context ownership via %s", source);
    }
}

// Pre-arms the menu ownership as soon as the pointer reaches a non-owner control-center region. Click-time transfer is too late : crash logs showed the lethal press dying inside the island's input processing before the taskbar window received any message, so the transfer must already be done when the hover or press input reaches the island.
void PreArmControlCenterContextOwnershipForTaskbar(HWND taskbarRootWnd, POINT screenPoint, PCWSTR source) {
    if (!taskbarRootWnd || taskbarRootWnd == g_controlCenterItemsOwnerTaskbarWnd || !IsTaskbarWindow(taskbarRootWnd)) {
        return;
    }

    // XAML islands must only be touched from their owning thread. Input for the taskbar islands is dispatched on the taskbar thread, so this only filters pathological senders.
    if (GetWindowThreadProcessId(taskbarRootWnd, nullptr) != GetCurrentThreadId()) {
        return;
    }

    POINT clientPoint = screenPoint;

    if (!ScreenToClient(taskbarRootWnd, &clientPoint) || !IsControlCenterClientPoint(taskbarRootWnd, clientPoint)) {
        return;
    }

    MoveControlCenterContextOwnershipToTaskbar(taskbarRootWnd, source);
}

/// Right-click/context handler on the taskbar window itself : logs the click breadcrumb and performs the last-resort ownership transfer when the hover/dispatch pre-arm did not already run
void PrepareControlCenterContextOwnership(HWND taskbarWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    POINT clientPoint = {};

    if (!TryGetTaskbarContextClientPoint(taskbarWnd, uMsg, wParam, lParam, &clientPoint) || !IsControlCenterClientPoint(taskbarWnd, clientPoint)) {
        return;
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : control center right-click/context "
            L"fall-through monitor=%d contextOwner=%d message=0x%04X "
            L"point=(%ld,%ld)",
            GetMonitorIndexForWindow(taskbarWnd),
            g_controlCenterItemsOwnerTaskbarWnd == taskbarWnd,
            uMsg, clientPoint.x, clientPoint.y
        );
    }

    MoveControlCenterContextOwnershipToTaskbar(taskbarWnd, L"right-click notification");
}

/// Every window message treated as right-click/context input, including the parent-level notification generated when a child (XAML island input window) receives the press
bool IsRightClickOrContextMenuMessage(UINT uMsg, WPARAM wParam) {
    return uMsg == WM_RBUTTONDOWN
        || uMsg == WM_RBUTTONUP
        || uMsg == WM_NCRBUTTONDOWN
        || uMsg == WM_NCRBUTTONUP
        || uMsg == WM_CONTEXTMENU
        || (uMsg == WM_PARENTNOTIFY && (LOWORD(wParam) == WM_RBUTTONDOWN || LOWORD(wParam) == WM_RBUTTONUP));
}

/// WM_POINTER messages that carry screen coordinates in lParam
bool IsPointerCoordinateMessage(UINT uMsg) {
    return uMsg == WM_POINTERDOWN
        || uMsg == WM_POINTERUP
        || uMsg == WM_POINTERUPDATE;
}

/// True for pointer messages with the second (right) button engaged
bool IsSecondButtonPointerMessage(UINT uMsg, WPARAM wParam) {
    return IsPointerCoordinateMessage(uMsg) && IS_POINTER_SECONDBUTTON_WPARAM(wParam);
}

/// Computes the screen anchor stored in a flyout context : the click point, or for hidden-tray clicks the chevron midpoint computed from the cached metrics, so the overflow flyout opens at the chevron even when the click landed in the hit slop
bool GetNativeFlyoutAnchorPoint(HWND hWnd, LPARAM lParam, WPARAM flyoutKind, POINT* anchorPoint) {
    RECT clientRect = {};

    if (!GetClientRect(hWnd, &clientRect)) {
        return false;
    }

    LONG clientX = GET_X_LPARAM(lParam);

    if (flyoutKind == kNativeHiddenTray) {
        UINT dpi = GetWindowDpiOrDefault(hWnd);
        int notificationAreaIconsWidth = ScaleTaskbarMetricForDpi(dpi, GetCachedNotificationAreaIconsWidth(hWnd));
        int hiddenTrayStart = ScaleTaskbarMetricForDpi(dpi, kShowDesktopWidth) + ScaleTaskbarMetricForDpi(dpi, kNotificationCenterWidth) + ScaleTaskbarMetricForDpi(dpi, kControlCenterWidth) + notificationAreaIconsWidth;
        int hiddenTrayWidth = ScaleTaskbarMetricForDpi(dpi, kNotifyIconStackWidth);
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

    if (GetSettings().enableVerboseLogging && flyoutKind == kNativeHiddenTray) {
        Wh_Log_safe(
            L"taskbar-multi-tray : anchored hidden tray flyout at "
            L"(%ld,%ld) for monitor %d",
            point.x, point.y, GetMonitorIndexForWindow(hWnd)
        );
    }

    return true;
}

/// Clears the armed flyout context locally and in the shared state, so both processes stop redirecting
void ClearProxyFlyoutMonitorState() {
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

    BeginSharedProxyStateWrite();
    g_sharedProxyState.proxyFlyoutMonitor = nullptr;
    g_sharedProxyState.proxyFlyoutUntilTick = 0;
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
    g_proxyFlyoutGeneration = EndSharedProxyStateWrite();
}

/// Drops an armed context when the user left-clicks outside the copied surfaces, so a stale monitor cannot drag the next flyout to the wrong screen
void ClearStaleFlyoutContextBeforeTaskbarClick(HWND taskbarWnd) {
    HMONITOR activeMonitor = GetActiveProxyFlyoutMonitor();

    if (!activeMonitor) {
        return;
    }

    HMONITOR taskbarMonitor = GetActualMonitorFromWindow(taskbarWnd, MONITOR_DEFAULTTONEAREST);

    if (!taskbarMonitor) {
        return;
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : clearing stale flyout monitor context "
            L"before non-target taskbar click monitor=%d active=%d",
            GetMonitorIndex(taskbarMonitor), GetMonitorIndex(activeMonitor)
        );
    }

    ClearProxyFlyoutMonitorState();
}

/// Drops an armed context when right-click/context input arrives : context menus must stay fully native and never inherit a left-click flyout's monitor redirection
void ClearFlyoutMonitorContextForContextMenu(UINT uMsg, WPARAM wParam, PCWSTR reason) {
    bool contextInput = IsRightClickOrContextMenuMessage(uMsg, wParam)
        || IsSecondButtonPointerMessage(uMsg, wParam);

    if (!contextInput || !GetActiveProxyFlyoutMonitor()) {
        return;
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : clearing flyout monitor context "
            L"before native right-click/context menu fall-through from %s "
            L"message=0x%04X",
            reason,
            uMsg
        );
    }

    ClearProxyFlyoutMonitorState();
}

/// Dispatch-loop guard : clears flyout contexts for right-click messages that travel through child/bridge HWNDs and would bypass the taskbar subclass
void InspectRetrievedMessageForFlyoutCancel(const MSG* msg, PCWSTR source) {
    if (!msg) {
        return;
    }

    ClearFlyoutMonitorContextForContextMenu(msg -> message, msg -> wParam, source);
}

// Runs in the dispatch path, before the target window procedure sees the message. This is the last safe moment to move the control-center menu ownership for hover and right-press input headed into a taskbar island : once the island starts processing a right press over a stale control-center region, Explorer is already dead. Mid-press containers are never re-attached (it would eat the active click), only pure hover moves and the right press itself, which has not been processed yet.
void PreArmControlCenterContextOwnershipForMessage(const MSG* msg) {
    // The ownership transfer only matters for the control-center per-glyph menus, skip the per-mouse-message work entirely when that surface is not managed
    if (!msg -> hwnd || !IsExplorerTarget() || !WantsControlCenter()) {
        return;
    }

    bool clientCoordinates;

    switch (msg -> message) {
        case WM_MOUSEMOVE:
            if (msg -> wParam & (MK_LBUTTON | MK_RBUTTON)) {
                return;
            }
            clientCoordinates = true;
            break;
        case WM_RBUTTONDOWN:
            clientCoordinates = true;
            break;
        case WM_POINTERUPDATE:
            if (IS_POINTER_FLAG_SET_WPARAM(msg -> wParam, POINTER_MESSAGE_FLAG_FIRSTBUTTON) || IS_POINTER_SECONDBUTTON_WPARAM(msg -> wParam)) {
                return;
            }
            clientCoordinates = false;
            break;
        case WM_POINTERDOWN:
            if (!IS_POINTER_SECONDBUTTON_WPARAM(msg -> wParam)) {
                return;
            }
            clientCoordinates = false;
            break;
        default:
            return;
    }

    HWND rootWnd = GetAncestor(msg -> hwnd, GA_ROOT);

    if (!rootWnd || rootWnd == g_controlCenterItemsOwnerTaskbarWnd) {
        return;
    }

    POINT screenPoint = {GET_X_LPARAM(msg -> lParam), GET_Y_LPARAM(msg -> lParam)};

    if (clientCoordinates && !ClientToScreen(msg -> hwnd, &screenPoint)) {
        return;
    }

    PreArmControlCenterContextOwnershipForTaskbar(rootWnd, screenPoint, L"input dispatch");
}

/// user32 hook on the message dispatch loop : clears flyout contexts on right-click input and pre-arms control-center menu ownership before the target window procedure runs
LRESULT WINAPI DispatchMessageW_Hook(const MSG* lpMsg) {
    if (!IsModUnloading() && lpMsg) {
        InspectRetrievedMessageForFlyoutCancel(lpMsg, L"DispatchMessageW");
        PreArmControlCenterContextOwnershipForMessage(lpMsg);
    }

    return DispatchMessageW_Original(lpMsg);
}

/// Resets per-session interaction state : the armed flyout context and the control-center ownership tracking
void ClearProxyRuntimeState() {
    ClearProxyFlyoutMonitorState();
    g_controlCenterItemsOwnerTaskbarWnd = nullptr;
}

/// Empties the heap-backed primary binding caches. Called on reload/settings paths only, never during process detach, so COM releases cannot run at an unsafe time.
void ClearCachedXamlBindings() {
    g_primaryNotificationAreaIconsBinding = {};
    g_primaryNotifyIconStackBinding = {};
    g_primaryNotifyIconStackChildBinding = {};
    g_primaryNotifyIconStackListViewBinding = {};
    g_primaryControlCenterButtonBinding = {};
}

/// Cached promoted-icon-area width (DIPs) for a taskbar, 0 when unknown
double GetCachedNotificationAreaIconsWidth(HWND hWnd) {
    for (const auto& metrics : g_taskbarTrayMetrics) {
        if (metrics.hWnd == hWnd) {
            return metrics.notificationAreaIconsWidth;
        }
    }

    return 0.0;
}

/// Stores the promoted-icon-area width measured during ApplyStyle, used by the click hit-testing
void SetCachedNotificationAreaIconsWidth(HWND hWnd, double width) {
    for (auto& metrics : g_taskbarTrayMetrics) {
        if (metrics.hWnd == hWnd) {
            metrics.notificationAreaIconsWidth = width;

            return;
        }
    }

    g_taskbarTrayMetrics.push_back({hWnd, width});
}

/// Drops the cached metrics of a destroyed taskbar window
void RemoveCachedTaskbarTrayMetrics(HWND hWnd) {
    for (auto it = g_taskbarTrayMetrics.begin(); it != g_taskbarTrayMetrics.end(); ++it) {
        if (it -> hWnd == hWnd) {
            g_taskbarTrayMetrics.erase(it);

            return;
        }
    }
}

/// Invalidates all pending deferred-apply retries by bumping the generation, and kills the active retry timer if any
void CancelDeferredApplySettings() {
    InterlockedIncrement(&g_deferredApplyGeneration);

    if (g_deferredApplyTimerWnd) {
        KillTimer(g_deferredApplyTimerWnd, kDeferredApplySettingsTimerId);
    }

    g_deferredApplyTimerWnd = nullptr;
    g_deferredApplyTimerGeneration = 0;
    g_deferredApplyRetryIndex = 0;
}

/// Kills the deferred-apply retry timer when this taskbar window owns it
void CancelTaskbarTimers(HWND hWnd) {
    if (!hWnd) {
        return;
    }

    if (g_deferredApplyTimerWnd == hWnd) {
        KillTimer(hWnd, kDeferredApplySettingsTimerId);
        g_deferredApplyTimerWnd = nullptr;
        g_deferredApplyTimerGeneration = 0;
        g_deferredApplyRetryIndex = 0;
    }
}

/// Arms the next bounded startup retry on the taskbar window, using the mod-unique timer id (small ids can collide with Explorer's own taskbar timers and must never be used)
/// @return false once the retry schedule is exhausted
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

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(
            L"taskbar-multi-tray : scheduled deferred apply retry %u "
            L"after %lu ms generation=%ld hwnd=0x%p",
            static_cast<unsigned>(g_deferredApplyRetryIndex + 1),
            delayMs, g_deferredApplyTimerGeneration, taskbarWnd
        );
    }

    return true;
}

/// Retry-timer callback : re-applies settings (the taskbar/XAML tree may have appeared late during startup) and schedules the next retry while the generation is still current
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

    if (GetSettings().enableVerboseLogging) {
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

/// Arms the clicked-monitor flyout context : monitor, kind, tick deadline, optional anchor, taskbar rectangle and target DPI, stored locally and published through the shared seqlock so ShellHost redirects too
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

    g_proxyFlyoutMonitor = monitor;
    g_proxyFlyoutMonitorIndex = monitorIndex;
    g_proxyFlyoutUntilTick = untilTick;
    g_proxyFlyoutKind = static_cast<int>(flyoutKind);
    g_proxyFlyoutTaskbarWnd = taskbarWnd;
    g_proxyFlyoutTargetDpi = targetDpi;

    BeginSharedProxyStateWrite();
    g_sharedProxyState.proxyFlyoutMonitor = monitor;
    g_sharedProxyState.proxyFlyoutMonitorIndex = monitorIndex;
    g_sharedProxyState.proxyFlyoutUntilTick = untilTick;
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

    g_proxyFlyoutGeneration = EndSharedProxyStateWrite();

    if (GetSettings().enableVerboseLogging) {
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

/// Arms a flyout context for a native control-center or hidden-tray click, with the kind-specific duration
void BeginNativeFlyoutMonitorContext(HWND taskbarWnd, PCWSTR reason, WPARAM flyoutKind, const POINT* anchorPoint) {
    DWORD durationMs = (flyoutKind == kNativeControlCenter)
        ? kNativeControlCenterMonitorContextMs
        : kNativeFlyoutMonitorContextMs;

    // No eager-clear timer on the taskbar window. The context expires lazily through proxyFlyoutUntilTick (checked by GetActiveProxyFlyoutMonitor), and SetTimer with a small id on Explorer's own taskbar windows could stomp a native Explorer timer, which the subclass would then swallow and kill : the prime suspect for the delayed right-click crashes on monitors 2/3.
    SetFlyoutMonitorContext(taskbarWnd, durationMs, reason, flyoutKind, anchorPoint);
}

/// Subclass installed on every managed taskbar window. Handles the deferred-apply timer, the WM_SETCURSOR hover pre-arm, right-click breadcrumbs plus last-resort ownership transfer, left-click flyout-context arming on the copied surfaces, stale-context clearing, and cleanup on WM_NCDESTROY. During unload everything passes through except timer/cleanup handling.
LRESULT CALLBACK TaskbarSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (IsModUnloading()) {
        if (uMsg == WM_TIMER && wParam == kDeferredApplySettingsTimerId) {
            CancelTaskbarTimers(hWnd);

            return 0;
        }

        if (uMsg == WM_NCDESTROY) {
            if (g_controlCenterItemsOwnerTaskbarWnd == hWnd) {
                g_controlCenterItemsOwnerTaskbarWnd = nullptr;
            }
            RemoveCachedTaskbarTrayMetrics(hWnd);
            CancelTaskbarTimers(hWnd);
            RemoveWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == WM_TIMER && wParam == kDeferredApplySettingsTimerId) {
        HandleDeferredApplySettingsTimer(hWnd);

        return 0;
    }

    if (uMsg == WM_DISPLAYCHANGE) {
        // Monitor topology changed : drop the cached monitor list so settings targeting and flyout contexts resolve against the new layout immediately instead of after the TTL
        InvalidateMonitorCache();

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    // Hover pre-arm : the island child forwards WM_SETCURSOR up the parent chain on every cursor move, which is the earliest taskbar-window signal that the pointer is approaching the control-center region. Moving the menu ownership here keeps the later press entirely on an island that already owns the menus. Never re-attach while a button is down, that would eat the in-flight click. Skipped entirely when the control-center surface is not managed (the ownership transfer only matters for its per-glyph menus).
    if (uMsg == WM_SETCURSOR && hWnd != g_controlCenterItemsOwnerTaskbarWnd && WantsControlCenter()) {
        POINT screenPoint = {};

        if (GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0 && GetCursorPos(&screenPoint)) {
            PreArmControlCenterContextOwnershipForTaskbar(hWnd, screenPoint, L"hover");
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (IsRightClickOrContextMenuMessage(uMsg, wParam)) {
        // Last-resort transfer plus breadcrumb logging. When the hover/dispatch pre-arm worked, the clicked taskbar already owns the menus and this only logs contextOwner=1.
        PrepareControlCenterContextOwnership(hWnd, uMsg, wParam, lParam);
        ClearFlyoutMonitorContextForContextMenu(uMsg, wParam, L"taskbar");

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    bool leftButtonDown = uMsg == WM_LBUTTONDOWN || (uMsg == WM_PARENTNOTIFY && LOWORD(wParam) == WM_LBUTTONDOWN);
    bool leftButtonUp = uMsg == WM_LBUTTONUP || (uMsg == WM_PARENTNOTIFY && LOWORD(wParam) == WM_LBUTTONUP);

    if (leftButtonDown || leftButtonUp) {
        WPARAM proxyFlyout = HitTestProxyFlyout(hWnd, lParam);

        if (proxyFlyout) {
            if (proxyFlyout == kNativeControlCenter) {
                if (leftButtonDown) {
                    BeginNativeFlyoutMonitorContext(hWnd, L"native control center", proxyFlyout, nullptr);
                }

                return DefSubclassProc(hWnd, uMsg, wParam, lParam);
            }

            if (proxyFlyout == kNativeHiddenTray) {
                if (leftButtonDown) {
                    POINT anchorPoint = {};
                    POINT* anchorPointPtr = GetNativeFlyoutAnchorPoint(hWnd, lParam, proxyFlyout, &anchorPoint)
                        ? &anchorPoint
                        : nullptr;
                    BeginNativeFlyoutMonitorContext(hWnd, L"native hidden tray", proxyFlyout, anchorPointPtr);
                }

                return DefSubclassProc(hWnd, uMsg, wParam, lParam);
            }
        } else if (leftButtonDown) {
            ClearStaleFlyoutContextBeforeTaskbarClick(hWnd);
        }
    } else if (uMsg == WM_NCDESTROY) {
        if (g_controlCenterItemsOwnerTaskbarWnd == hWnd) {
            g_controlCenterItemsOwnerTaskbarWnd = nullptr;
        }
        RemoveCachedTaskbarTrayMetrics(hWnd);
        CancelTaskbarTimers(hWnd);
        RemoveWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId);
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

/// Installs TaskbarSubclassProc on a taskbar window (logged on failure)
void InstallTaskbarSubclass(HWND hWnd) {
    if (!SetWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId, 0)) {
        Wh_Log_safe(
            L"taskbar-multi-tray : SetWindowSubclass failed for hwnd=0x%p "
            L"error=%lu",
            hWnd, GetLastError()
        );
    }
}

/// Cancels this taskbar's timers and removes the subclass
void RemoveTaskbarSubclass(HWND hWnd) {
    CancelTaskbarTimers(hWnd);
    RemoveWindowSubclass(hWnd, TaskbarSubclassProc, kTaskbarSubclassId);
}

/// Extracts the XamlRoot from a TaskbarHost std::shared_ptr without knowing the class layout : the offset of the hosted XAML element is read out of TaskbarHost::FrameHeight's prologue bytes (x64 and ARM64 encodings below), then the shared_ptr's control block is released via std::_Ref_count_base::_Decref
XamlRoot XamlRootFromTaskbarHostSharedPtr(void* taskbarHostSharedPtr[2]) {
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1]) {
        return nullptr;
    }

    size_t taskbarElementIUnknownOffset = 0x48;

// https://github.com/m417z/my-windhawk-mods/blob/544ca828ae800f84183df7439194d2abb5ef7f03/mods/taskbar-notification-icon-spacing.wh.cpp#L801-L871
#if defined(_M_X64)
{
    // 48:83EC 28 | sub rsp,28
    // 48:83C1 48 | add rcx,48
    const BYTE* b = static_cast<const BYTE*>(TaskbarHost_FrameHeight_Original);

    if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
        b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F) {
        taskbarElementIUnknownOffset = b[7];
    } else {
        Wh_Log_safe(L"taskbar-multi-tray : unsupported TaskbarHost::FrameHeight");
    }
}
#elif defined(_M_ARM64)
{
    // 7f2303d5 pacibsp
    // fd7bbfa9 stp     fp, lr, [sp, #-0x10]!
    // fd030091 mov     fp, sp
    // 080c41f8 ldr     x8, [x0, #0x10]!
    const DWORD* p = static_cast<const DWORD*>(TaskbarHost_FrameHeight_Original);
    if (p[0] == 0xD503237F && (p[1] & 0xFFC07FFF) == 0xA9807BFD &&
        p[2] == 0x910003FD && (p[3] & 0xFFF00FE0) == 0xF8400C00) {
        taskbarElementIUnknownOffset = (p[3] >> 12) & 0xFF;
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

/// XamlRoot of the primary taskbar : finds CTaskBand through the window's ITaskListWndSite vtable slots, then asks it for its TaskbarHost
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

/// TaskbarHost shared_ptr of a secondary taskbar via CSecondaryTaskBand, same vtable-walk approach as the primary
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

/// XamlRoot of a secondary taskbar
XamlRoot GetSecondaryTaskbarXamlRoot(HWND secondaryTaskbarWnd) {
    void* taskbarHostSharedPtr[2]{};

    if (!GetSecondaryTaskbarHostSharedPtr(secondaryTaskbarWnd, taskbarHostSharedPtr)) {
        return nullptr;
    }

    return XamlRootFromTaskbarHostSharedPtr(taskbarHostSharedPtr);
}

/// SystemTrayFrameGrid element of any taskbar window, primary or secondary : the root of every tray surface the mod touches
FrameworkElement FindSystemTrayFrameGridForTaskbar(HWND taskbarWnd) {
    XamlRoot xamlRoot = IsSecondaryTaskbarWindow(taskbarWnd)
        ? GetSecondaryTaskbarXamlRoot(taskbarWnd)
        : GetTaskbarXamlRoot(taskbarWnd);

    if (!xamlRoot) {
        return nullptr;
    }

    FrameworkElement child = xamlRoot.Content().try_as<FrameworkElement>();

    if (!child) {
        return nullptr;
    }

    child = FindChildByClassName(child, L"SystemTray.SystemTrayFrame");

    if (!child) {
        return nullptr;
    }

    return FindChildByName(child, L"SystemTrayFrameGrid");
}

/// The full apply pass, always on the taskbar thread : installs the flyout ShowAt vtable hooks, enumerates this thread's taskbar windows, styles the real-tray owner first (so its bindings are cached before any copy consumes them), then styles the rest and installs subclasses
void ApplySettingsFromTaskbarThread(void*) {
    if (IsModUnloading()) {
        return;
    }

    ActiveFlyoutRedirectionSuppressor suppressActiveFlyoutRedirection;

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : applying from thread %lu", GetCurrentThreadId());
    }

    // The taskbar thread owns the XAML core, the only place the flyout ShowAt vtable hooks can be installed
    EnsureFlyoutShowAtVtableHooks();

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

/// Unload helper on the taskbar thread : unpatches the flyout vtables and removes every taskbar subclass and timer
void RemoveTaskbarSubclassesFromTaskbarThread(void*) {
    // Unpatch the flyout ShowAt vtable slots on the same thread that runs them, before the mod code can go away
    RemoveFlyoutShowAtVtableHooks();

    EnumThreadWindows(
        GetCurrentThreadId(),
        [](HWND hWnd, LPARAM) -> BOOL {
            wchar_t className[32] = {};

            if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) && (_wcsicmp(className, L"Shell_TrayWnd") == 0 || _wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0)) {
                RemoveTaskbarSubclass(hWnd);

                if (GetSettings().enableVerboseLogging) {
                    Wh_Log_safe(L"taskbar-multi-tray : removed subclass/timers from taskbar hwnd=0x%p monitor=%d", hWnd, GetMonitorIndexForWindow(hWnd));
                }
            }

            return TRUE;
        },
        0
    );
}

/// Unload helper on the taskbar thread : restores the primary taskbar to native primary state and every secondary to the default secondary state
void RestoreNativeTaskbarsFromTaskbarThread(void*) {
    ActiveFlyoutRedirectionSuppressor suppressActiveFlyoutRedirection;

    if (GetSettings().enableVerboseLogging) {
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

using RunFromWindowThreadProc_t = void(WINAPI*)(void* parameter);

/// Runs a callback synchronously on a window's owning thread : a WH_CALLWNDPROC hook intercepts a registered message sent with SendMessageTimeout(SMTO_ABORTIFHUNG), so a hung Explorer can never deadlock the unload path. Runs inline when already on the right thread.
bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc, void* procParam) {
    static const UINT runFromWindowThreadRegisteredMsg =
        RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);

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
        if (GetSettings().enableVerboseLogging) {
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

                if (cwp->message == runFromWindowThreadRegisteredMsg) {
                    auto* param = reinterpret_cast<RunFromWindowThreadParam*>(cwp->lParam);
                    param->proc(param->procParam);
                }
            }

            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        },
        nullptr,
        threadId
    );

    if (!hook) {
        Wh_Log_safe(
            L"taskbar-multi-tray : SetWindowsHookEx failed for thread %lu, error=%lu",
            threadId,
            GetLastError()
        );
        return false;
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : dispatching to taskbar thread %lu", threadId);
    }

    RunFromWindowThreadParam param{proc, procParam};
    LRESULT messageResult = SendMessageTimeout(
        hWnd,
        runFromWindowThreadRegisteredMsg,
        0,
        reinterpret_cast<LPARAM>(&param),
        SMTO_ABORTIFHUNG,
        2000,
        nullptr
    );

    UnhookWindowsHookEx(hook);

    if (!messageResult) {
        Wh_Log_safe(
            L"taskbar-multi-tray : SendMessageTimeout failed for taskbar thread %lu hwnd=0x%p error=%lu",
            threadId,
            hWnd,
            GetLastError()
        );
        return false;
    }

    return true;
}

/// Synchronous apply entry point : finds the taskbar and dispatches the apply pass to its thread
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

/// Starts the bounded deferred retry schedule, covering Explorer startups where the taskbar/XAML tree is created after the mod loads
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
/// taskbar.dll hook : TrayUI::StartTaskbar is the first reliable moment the taskbar exists, so settings are applied and the startup retries armed right after it
void WINAPI TrayUI_StartTaskbar_Hook(void* pThis) {
    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : TrayUI::StartTaskbar hook");
    }

    TrayUI_StartTaskbar_Original(pThis);

    if (IsModUnloading()) {
        return;
    }

    ApplySettingsFromTaskbarThread(nullptr);
    QueueDeferredApplySettings();
}

using CSecondaryTray_GetTrayWindow_t = HWND(WINAPI*)(void* pThis);
CSecondaryTray_GetTrayWindow_t CSecondaryTray_GetTrayWindow_Original;

using CSecondaryTray_InitModelAndHost_t = void(WINAPI*)(void* pThis, void* taskbarModel);
CSecondaryTray_InitModelAndHost_t CSecondaryTray_InitModelAndHost_Original;
/// taskbar.dll hook : styles a secondary taskbar right after Windows creates it (hot-plug or late creation). The initialization itself is left fully native.
void WINAPI CSecondaryTray_InitModelAndHost_Hook(void* pThis, void* taskbarModel) {
    if (IsModUnloading()) {
        CSecondaryTray_InitModelAndHost_Original(pThis, taskbarModel);

        return;
    }

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : CSecondaryTray::InitModelAndHost hook");
    }

    HWND taskbarWnd = CSecondaryTray_GetTrayWindow_Original(pThis);

    // Let the secondary tray initialize fully natively. Earlier versions spoofed monitor/primary-display queries here and forced a primary-like TaskbarHost flag, which attached extra primary-like hosts to Windows' process-wide singleton tray model. The singleton's control-center context-menu wiring follows exactly one host, so the last spoofed host to attach (the laptop taskbar in the captured logs) hijacked it away from the real primary taskbar : right-clicking control-center icons then crashed Explorer on every other monitor, including the otherwise untouched native primary.
    CSecondaryTray_InitModelAndHost_Original(pThis, taskbarModel);

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

/// taskbar.dll hook : whenever Explorer re-evaluates where the primary taskbar lives, selected mode retargets the singleton real-tray surface to the preferred monitor (and the restore path sends it back)
HRESULT WINAPI TrayUI__SetStuckMonitor_Hook(void* pThis, HMONITOR monitor) {
    if (IsModUnloading() && !g_restoringNativeTaskbars) {
        return TrayUI__SetStuckMonitor_Original(pThis, monitor);
    }

    HMONITOR targetMonitor = GetRealPrimaryTrayTargetMonitor();

    if (targetMonitor) {
        if (GetSettings().enableVerboseLogging) {
            Wh_Log_safe(
                L"taskbar-multi-tray : %s real primary tray to monitor %d",
                g_restoringNativeTaskbars ? L"restoring" : L"moving",
                GetMonitorIndex(targetMonitor)
            );
        }

        monitor = targetMonitor;
    } else if (GetSettings().monitorMode == MonitorMode::All && GetSettings().enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : monitorMode=all can't duplicate the real primary tray model; applying XAML visibility/layout only");
    }

    return TrayUI__SetStuckMonitor_Original(pThis, monitor);
}

/// Resolves and hooks every required taskbar.dll symbol in a single HookSymbols pass : if any symbol drifted in a Windows update the mod refuses to load instead of half-loading
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
            {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
            &std__Ref_count_base__Decref_Original,
        },
        {
            {LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"},
            &TrayUI_StartTaskbar_Original,
            TrayUI_StartTaskbar_Hook,
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

    return true;
}

/// Hooks the immersive monitor helper in twinui.pcshell.dll. Optional : flyout placement still mostly works through the user32 hooks when these symbols are unavailable.
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

/// Mod entry point : detects the host process, loads settings, installs symbol hooks (Explorer only) and the shared user32 hooks
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

/// Post-hook initialization : applies settings and arms the startup retries (Explorer only)
void Wh_ModAfterInit() {
    Wh_Log_safe(L"taskbar-multi-tray : after init");

    if (IsExplorerTarget()) {
        ApplySettings();
        QueueDeferredApplySettings();
    }
}

/// Ordered teardown : flags unloading so every hook goes pass-through, cancels retries, clears state, then removes subclasses and restores native taskbar state from the taskbar's own thread
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

    g_restoringNativeTaskbars = true;
    g_nativePrimaryRestoreMonitor = GetActualMonitorFromWindow(taskbarWnd, MONITOR_DEFAULTTONEAREST);

    if (GetSettings().enableVerboseLogging) {
        Wh_Log_safe(L"taskbar-multi-tray : native restore target monitor=%d", GetMonitorIndex(g_nativePrimaryRestoreMonitor));
    }

    RunFromWindowThread(taskbarWnd, RemoveTaskbarSubclassesFromTaskbarThread, nullptr);
    NotifyTaskbarDisplayChange(taskbarWnd);
    RunFromWindowThread(taskbarWnd, RestoreNativeTaskbarsFromTaskbarThread, nullptr);
    RunFromWindowThread(taskbarWnd, RemoveTaskbarSubclassesFromTaskbarThread, nullptr);

    g_restoringNativeTaskbars = false;
    g_nativePrimaryRestoreMonitor = nullptr;
}

/// Settings change : reload, reset caches and interaction state, then re-apply
void Wh_ModSettingsChanged() {
    Wh_Log_safe(L"taskbar-multi-tray : settings changed");
    LoadSettings();
    CancelDeferredApplySettings();
    ClearProxyRuntimeState();
    ClearCachedXamlBindings();

    if (IsExplorerTarget()) {
        ApplySettings();
        QueueDeferredApplySettings();
    }
}
