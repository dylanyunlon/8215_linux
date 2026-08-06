import org.gradle.api.invocation.Gradle

class ConfigUtils {
    /** 初始化工程配置 */
    static init(Gradle gradle) {
        generateDepends(gradle)
        if (gradle.getStartParameter().getTaskNames().toString() != "[clean]") {
            DurationUtils.init(gradle)
        }
    }

    /**
     * 根据 depConfig 生成 dep
     */
    private static void generateDepends(Gradle gradle) {
        def configs = [:]
        for (Map.Entry<String, ModuleConfig> entry : BuildConfig.modules.entrySet()) {
            def (name, config) = [entry.key, entry.value]
            if (config.useLocal) {
                config.dep = gradle.rootProject.findProject(name)
            } else {
                config.dep = config.remotePath
            }

            configs.put(name, config)
        }

        GLog.l("generateDepends = ${GLog.object2String(configs)}")
    }

    static getApplyPlugins() {
        def plugins = [:]
        for (Map.Entry<String, PluginConfig> entry : BuildConfig.plugins.entrySet()) {
            if (entry.value.isApply) {
                plugins.put(entry.key, entry.value)
            }
        }

        GLog.d("getApplyPlugins = ${GLog.object2String(plugins)}")
        return plugins
    }

    static getApplyPkgs() {
        def pkgs = [:]
        for (Map.Entry<String, ModuleConfig> entry : BuildConfig.modules.entrySet()) {
            if (entry.value.isApply && entry.key.endsWith("_pkg")) {
                pkgs.put(entry.key, entry.value)
            }
        }

        GLog.d("getApplyPkgs = ${GLog.object2String(pkgs)}")
        return pkgs
    }

    static getApplyExports() {
        def exports = [:]
        for (Map.Entry<String, ModuleConfig> entry : BuildConfig.modules.entrySet()) {
            if (entry.value.isApply && entry.key.endsWith("_export")) {
                exports.put(entry.key, entry.value)
            }
        }

        GLog.d("getApplyExports = ${GLog.object2String(exports)}")
        return exports
    }
}
