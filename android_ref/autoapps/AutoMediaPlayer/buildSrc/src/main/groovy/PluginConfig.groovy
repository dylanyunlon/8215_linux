/**
 * 插件配置
 * <p> 用来管理项目构建中引入的各种插件和支持库；
 *
 * @author 86158
 */
final class PluginConfig {
    boolean isApply = true  // 是否应用
    boolean useLocal        // 是否使用本地的
    String path             // 插件路径
    String id               // 插件 ID

    String getGroupId() {
        String[] splits = path.split(":")
        return splits.length == 3 ? splits[0] : null
    }

    String getArtifactId() {
        String[] splits = path.split(":")
        return splits.length == 3 ? splits[1] : null
    }

    String getVersion() {
        String[] splits = path.split(":")
        return splits.length == 3 ? splits[2] : null
    }

    @Override
    String toString() {
        return "PluginConfig { isApply = ${getFlag(isApply)}" +
                ", useLocal = ${getFlag(useLocal)}" +
                ", path = " + path +
                ", id = " + id +
                " }"
    }

    static String getFlag(boolean b) {
        return b ? "✅" : "❌"
    }
}