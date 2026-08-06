import org.gradle.BuildListener
import org.gradle.BuildResult
import org.gradle.api.Task
import org.gradle.api.execution.TaskExecutionListener
import org.gradle.api.initialization.Settings
import org.gradle.api.invocation.Gradle
import org.gradle.api.tasks.TaskState

import java.text.SimpleDateFormat

/**
 * 任务持续工具
 * <p> 统计记录任务执行时间；
 *
 * @author 86158
 */
class DurationUtils {
    static List<TaskInfo> taskInfoList = []
    static long startMillis

    static init(Gradle grd) {
        startMillis = System.currentTimeMillis()

        grd.addListener(new TaskExecutionListener() {
            @Override
            void beforeExecute(Task task) {
                task.ext.startTime = System.currentTimeMillis()
            }

            @Override
            void afterExecute(Task task, TaskState state) {
                def exeDuration = System.currentTimeMillis() - task.ext.startTime
                if (exeDuration >= 500) {
                    taskInfoList.add(new TaskInfo(task: task, exeDuration: exeDuration))
                }
            }
        })

        grd.addBuildListener(new BuildListener() {
            @Override
            void beforeSettings(Settings settings) {
                super.beforeSettings(settings)
            }

            @Override
            void settingsEvaluated(Settings settings) {}

            @Override
            void projectsLoaded(Gradle gradle) {}

            @Override
            void projectsEvaluated(Gradle gradle) {}

            @Override
            void buildFinished(BuildResult buildResult) {
                if (!taskInfoList.isEmpty()) {
                    Collections.sort(taskInfoList, new Comparator<TaskInfo>() {
                        @Override
                        int compare(TaskInfo t, TaskInfo t1) {
                            return t1.exeDuration - t.exeDuration
                        }
                    })

                    StringBuilder sb = new StringBuilder()
                    int buildSec = (System.currentTimeMillis() - startMillis) / 1000;
                    int m = buildSec / 60;
                    int s = buildSec % 60;
                    def timeInfo = (m == 0 ? "${s}s" : "${m}m ${s}s (${buildSec}s)")
                    sb.append("BUILD FINISHED in $timeInfo\n")
                    taskInfoList.each {
                        sb.append(String.format("%7sms %s\n", it.exeDuration, it.task.path))
                    }

                    def content = sb.toString()
                    GLog.d(content)
                    String path = grd.rootProject.getRootDir().getAbsolutePath()
                    File file = new File(path + "/.records",
                            "build_time_records_" + new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss").format(new Date()) + ".txt")
                    file.getParentFile().mkdirs()
                    file.write(content)
                }
            }
        })
    }

    private static class TaskInfo {
        Task task
        long exeDuration
    }
}
