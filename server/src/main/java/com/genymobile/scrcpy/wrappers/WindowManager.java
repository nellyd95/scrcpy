package com.genymobile.scrcpy.wrappers;

import com.genymobile.scrcpy.AndroidVersions;
import com.genymobile.scrcpy.util.Ln;
import com.genymobile.scrcpy.util.Command;

import android.annotation.TargetApi;
import android.os.Build;
import android.os.IInterface;
import android.view.IDisplayWindowListener;

import java.lang.reflect.Method;

public final class WindowManager {

    @SuppressWarnings("checkstyle:LineLength")
    // <https://android.googlesource.com/platform/frameworks/base.git/+/2103ff441c66772c80c8560e322dcd9a45be7dcd/core/java/android/view/WindowManager.java#692>
    public static final int DISPLAY_IME_POLICY_LOCAL = 0;
    public static final int DISPLAY_IME_POLICY_FALLBACK_DISPLAY = 1;
    public static final int DISPLAY_IME_POLICY_HIDE = 2;

    // IWindowManager fixed-to-user-rotation policy constants (Android 11+).
    // Unlike freezeDisplayRotation(), ENABLED makes Android honor all four
    // user rotation angles even on devices normally restricting app rotation.
    public static final int FIXED_TO_USER_ROTATION_DEFAULT = 0;
    public static final int FIXED_TO_USER_ROTATION_DISABLED = 1;
    public static final int FIXED_TO_USER_ROTATION_ENABLED = 2;

    private final IInterface manager;
    private Method getRotationMethod;

    private Method freezeDisplayRotationMethod;
    private int freezeDisplayRotationMethodVersion;

    private Method isDisplayRotationFrozenMethod;
    private int isDisplayRotationFrozenMethodVersion;

    private Method thawDisplayRotationMethod;
    private int thawDisplayRotationMethodVersion;

    private Method getFixedToUserRotationMethod;
    private Method setFixedToUserRotationMethod;
    private Method getDisplayUserRotationMethod;

    private Method getDisplayImePolicyMethod;
    private Method setDisplayImePolicyMethod;

    static WindowManager create() {
        IInterface manager = ServiceManager.getService("window", "android.view.IWindowManager");
        return new WindowManager(manager);
    }

    private WindowManager(IInterface manager) {
        this.manager = manager;
    }

    private Method getGetRotationMethod() throws NoSuchMethodException {
        if (getRotationMethod == null) {
            Class<?> cls = manager.getClass();
            try {
                // method changed since this commit:
                // https://android.googlesource.com/platform/frameworks/base/+/8ee7285128c3843401d4c4d0412cd66e86ba49e3%5E%21/#F2
                getRotationMethod = cls.getMethod("getDefaultDisplayRotation");
            } catch (NoSuchMethodException e) {
                // old version
                getRotationMethod = cls.getMethod("getRotation");
            }
        }
        return getRotationMethod;
    }

    private Method getFreezeDisplayRotationMethod() throws NoSuchMethodException {
        if (freezeDisplayRotationMethod == null) {
            try {
                // Android 15 preview and 14 QPR3 Beta added a String caller parameter for debugging:
                // <https://android.googlesource.com/platform/frameworks/base/+/670fb7f5c0d23cf51ead25538bcb017e03ed73ac%5E%21/>
                freezeDisplayRotationMethod = manager.getClass().getMethod("freezeDisplayRotation", int.class, int.class, String.class);
                freezeDisplayRotationMethodVersion = 0;
            } catch (NoSuchMethodException e) {
                try {
                    // New method added by this commit:
                    // <https://android.googlesource.com/platform/frameworks/base/+/90c9005e687aa0f63f1ac391adc1e8878ab31759%5E%21/>
                    freezeDisplayRotationMethod = manager.getClass().getMethod("freezeDisplayRotation", int.class, int.class);
                    freezeDisplayRotationMethodVersion = 1;
                } catch (NoSuchMethodException e1) {
                    freezeDisplayRotationMethod = manager.getClass().getMethod("freezeRotation", int.class);
                    freezeDisplayRotationMethodVersion = 2;
                }
            }
        }
        return freezeDisplayRotationMethod;
    }

    private Method getIsDisplayRotationFrozenMethod() throws NoSuchMethodException {
        if (isDisplayRotationFrozenMethod == null) {
            try {
                // New method added by this commit:
                // <https://android.googlesource.com/platform/frameworks/base/+/90c9005e687aa0f63f1ac391adc1e8878ab31759%5E%21/>
                isDisplayRotationFrozenMethod = manager.getClass().getMethod("isDisplayRotationFrozen", int.class);
                isDisplayRotationFrozenMethodVersion = 0;
            } catch (NoSuchMethodException e) {
                isDisplayRotationFrozenMethod = manager.getClass().getMethod("isRotationFrozen");
                isDisplayRotationFrozenMethodVersion = 1;
            }
        }
        return isDisplayRotationFrozenMethod;
    }

    private Method getThawDisplayRotationMethod() throws NoSuchMethodException {
        if (thawDisplayRotationMethod == null) {
            try {
                // Android 15 preview and 14 QPR3 Beta added a String caller parameter for debugging:
                // <https://android.googlesource.com/platform/frameworks/base/+/670fb7f5c0d23cf51ead25538bcb017e03ed73ac%5E%21/>
                thawDisplayRotationMethod = manager.getClass().getMethod("thawDisplayRotation", int.class, String.class);
                thawDisplayRotationMethodVersion = 0;
            } catch (NoSuchMethodException e) {
                try {
                    // New method added by this commit:
                    // <https://android.googlesource.com/platform/frameworks/base/+/90c9005e687aa0f63f1ac391adc1e8878ab31759%5E%21/>
                    thawDisplayRotationMethod = manager.getClass().getMethod("thawDisplayRotation", int.class);
                    thawDisplayRotationMethodVersion = 1;
                } catch (NoSuchMethodException e1) {
                    thawDisplayRotationMethod = manager.getClass().getMethod("thawRotation");
                    thawDisplayRotationMethodVersion = 2;
                }
            }
        }
        return thawDisplayRotationMethod;
    }

    public int getRotation() {
        try {
            Method method = getGetRotationMethod();
            return (int) method.invoke(manager);
        } catch (ReflectiveOperationException e) {
            Ln.e("Could not invoke method", e);
            return 0;
        }
    }

    public void freezeRotation(int displayId, int rotation) {
        try {
            Method method = getFreezeDisplayRotationMethod();
            switch (freezeDisplayRotationMethodVersion) {
                case 0:
                    method.invoke(manager, displayId, rotation, "scrcpy#freezeRotation");
                    break;
                case 1:
                    method.invoke(manager, displayId, rotation);
                    break;
                default:
                    if (displayId != 0) {
                        Ln.e("Secondary display rotation not supported on this device");
                        return;
                    }
                    method.invoke(manager, rotation);
                    break;
            }
        } catch (ReflectiveOperationException e) {
            Ln.e("Could not invoke method", e);
        }
    }

    public boolean isRotationFrozen(int displayId) {
        try {
            Method method = getIsDisplayRotationFrozenMethod();
            switch (isDisplayRotationFrozenMethodVersion) {
                case 0:
                    return (boolean) method.invoke(manager, displayId);
                default:
                    if (displayId != 0) {
                        Ln.e("Secondary display rotation not supported on this device");
                        return false;
                    }
                    return (boolean) method.invoke(manager);
            }
        } catch (ReflectiveOperationException e) {
            Ln.e("Could not invoke method", e);
            return false;
        }
    }

    public void thawRotation(int displayId) {
        try {
            Method method = getThawDisplayRotationMethod();
            switch (thawDisplayRotationMethodVersion) {
                case 0:
                    method.invoke(manager, displayId, "scrcpy#thawRotation");
                    break;
                case 1:
                    method.invoke(manager, displayId);
                    break;
                default:
                    if (displayId != 0) {
                        Ln.e("Secondary display rotation not supported on this device");
                        return;
                    }
                    method.invoke(manager);
                    break;
            }
        } catch (ReflectiveOperationException e) {
            Ln.e("Could not invoke method", e);
        }
    }

    // Some Android vendors expose the setter through the IWindowManager
    // binder proxy but not the corresponding getter. The Android shell
    // command uses WindowManagerInternal and is therefore the compatible
    // way to capture the original policy before any modification.
    private static String readWindowCommand(String... args) {
        try {
            String[] command = new String[args.length + 2];
            command[0] = "/system/bin/cmd";
            command[1] = "window";
            System.arraycopy(args, 0, command, 2, args.length);
            return Command.execReadLine(command);
        } catch (Exception e) {
            Ln.w("Could not read Android window-manager state: " + e);
            return null;
        }
    }

    public int getFixedToUserRotation(int displayId) {
        String result = readWindowCommand("fixed-to-user-rotation",
                "-d", String.valueOf(displayId));
        if ("default".equals(result)) {
            return FIXED_TO_USER_ROTATION_DEFAULT;
        }
        if ("disabled".equals(result)) {
            return FIXED_TO_USER_ROTATION_DISABLED;
        }
        if ("enabled".equals(result)) {
            return FIXED_TO_USER_ROTATION_ENABLED;
        }
        if ("enabled_if_no_auto_rotation".equals(result)) {
            return 3;
        }
        Ln.w("Unexpected fixed-to-user-rotation response: " + result);
        return -1;
    }

    public boolean setFixedToUserRotation(int displayId, int mode) {
        if (mode < 0 || mode > 3) {
            return false;
        }

        try {
            if (setFixedToUserRotationMethod == null) {
                setFixedToUserRotationMethod = manager.getClass().getMethod(
                        "setFixedToUserRotation", int.class, int.class);
            }
            setFixedToUserRotationMethod.invoke(manager, displayId, mode);
            return true;
        } catch (ReflectiveOperationException | SecurityException e) {
            // Android ROMs have differing IWindowManager proxy methods.
            // Use the same shell command as adb shell wm if unavailable.
            final String option;
            switch (mode) {
                case FIXED_TO_USER_ROTATION_DEFAULT:
                    option = "default";
                    break;
                case FIXED_TO_USER_ROTATION_DISABLED:
                    option = "disabled";
                    break;
                case FIXED_TO_USER_ROTATION_ENABLED:
                    option = "enabled";
                    break;
                default:
                    option = "enabled_if_no_auto_rotation";
                    break;
            }
            try {
                Command.exec("/system/bin/cmd", "window", "fixed-to-user-rotation",
                        "-d", String.valueOf(displayId), option);
                return true;
            } catch (Exception fallback) {
                Ln.e("Could not change fixed-to-user-rotation policy", fallback);
                return false;
            }
        }
    }

    public int getDisplayUserRotation(int displayId) {
        String state = readWindowCommand("user-rotation",
                "-d", String.valueOf(displayId));
        if (state != null && state.startsWith("lock ")) {
            try {
                int value = Integer.parseInt(state.substring(5).trim());
                return value >= 0 && value <= 3 ? value : -1;
            } catch (NumberFormatException e) {
                Ln.w("Invalid user-rotation response: " + state);
            }
        }
        if ("free".equals(state)) {
            // The prior display user angle is irrelevant when its original
            // rotation mode was free. A valid fallback still allows us to
            // restore free mode without discarding the original fixed policy.
            return getRotation();
        }
        Ln.w("Could not determine original user rotation: " + state);
        return -1;
    }

    @TargetApi(AndroidVersions.API_30_ANDROID_11)
    public int[] registerDisplayWindowListener(IDisplayWindowListener listener) {
        try {
            return (int[]) manager.getClass().getMethod("registerDisplayWindowListener", IDisplayWindowListener.class).invoke(manager, listener);
        } catch (Exception e) {
            Ln.e("Could not register display window listener", e);
        }
        return null;
    }

    @TargetApi(AndroidVersions.API_30_ANDROID_11)
    public void unregisterDisplayWindowListener(IDisplayWindowListener listener) {
        try {
            manager.getClass().getMethod("unregisterDisplayWindowListener", IDisplayWindowListener.class).invoke(manager, listener);
        } catch (Exception e) {
            Ln.e("Could not unregister display window listener", e);
        }
    }

    @TargetApi(AndroidVersions.API_29_ANDROID_10)
    private Method getGetDisplayImePolicyMethod() throws NoSuchMethodException {
        if (getDisplayImePolicyMethod == null) {
            if (Build.VERSION.SDK_INT >= AndroidVersions.API_31_ANDROID_12) {
                getDisplayImePolicyMethod = manager.getClass().getMethod("getDisplayImePolicy", int.class);
            } else {
                getDisplayImePolicyMethod = manager.getClass().getMethod("shouldShowIme", int.class);
            }
        }
        return getDisplayImePolicyMethod;
    }

    @TargetApi(AndroidVersions.API_29_ANDROID_10)
    public int getDisplayImePolicy(int displayId) {
        try {
            Method method = getGetDisplayImePolicyMethod();
            if (Build.VERSION.SDK_INT >= AndroidVersions.API_31_ANDROID_12) {
                return (int) method.invoke(manager, displayId);
            }
            boolean shouldShowIme = (boolean) method.invoke(manager, displayId);
            return shouldShowIme ? DISPLAY_IME_POLICY_LOCAL : DISPLAY_IME_POLICY_FALLBACK_DISPLAY;
        } catch (ReflectiveOperationException e) {
            Ln.e("Could not invoke method", e);
            return -1;
        }
    }

    @TargetApi(AndroidVersions.API_29_ANDROID_10)
    private Method getSetDisplayImePolicyMethod() throws NoSuchMethodException {
        if (setDisplayImePolicyMethod == null) {
            if (Build.VERSION.SDK_INT >= AndroidVersions.API_31_ANDROID_12) {
                setDisplayImePolicyMethod = manager.getClass().getMethod("setDisplayImePolicy", int.class, int.class);
            } else {
                setDisplayImePolicyMethod = manager.getClass().getMethod("setShouldShowIme", int.class, boolean.class);
            }
        }
        return setDisplayImePolicyMethod;
    }

    @TargetApi(AndroidVersions.API_29_ANDROID_10)
    public void setDisplayImePolicy(int displayId, int displayImePolicy) {
        try {
            Method method = getSetDisplayImePolicyMethod();
            if (Build.VERSION.SDK_INT >= AndroidVersions.API_31_ANDROID_12) {
                method.invoke(manager, displayId, displayImePolicy);
            } else if (displayImePolicy != DISPLAY_IME_POLICY_HIDE) {
                method.invoke(manager, displayId, displayImePolicy == DISPLAY_IME_POLICY_LOCAL);
            } else {
                Ln.w("DISPLAY_IME_POLICY_HIDE is not supported before Android 12");
            }
        } catch (ReflectiveOperationException e) {
            Ln.e("Could not invoke method", e);
        }
    }
}
