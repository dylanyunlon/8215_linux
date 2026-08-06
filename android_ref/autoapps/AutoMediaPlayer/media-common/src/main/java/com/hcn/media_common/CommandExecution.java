package com.hcn.media_common;

import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * 执行shell脚本工具类
 *
 * @Author youwj
 * @Create 2021/8/6 12:16
 */
public class CommandExecution {
    public static final String TAG = "CommandExecution";

    public final static String COMMAND_SU       = "su";
    public final static String COMMAND_SH       = "sh";
    public final static String COMMAND_EXIT     = "exit\n";
    public final static String COMMAND_LINE_END = "\n";

    /**
     * Command 执行结果
     */
    public static class CommandResult {
        public int result = -1;
        public String errorMsg;
        public String successMsg;
    }

    /**
     * 执行命令—单条
     * @param command 命令
     * @param root 是否 root
     * @return 执行结果
     */
    public static CommandResult execCommand(String command, boolean root) {
        String[] commands = {command};
        return execCommand(commands, root);
    }

    /**
     * 执行命令-多条
     * @param commands 命令集
     * @param root 是否 root
     * @return 执行结果
     */
    public static CommandResult execCommand(String[] commands, boolean root) {
        CommandResult commandResult = new CommandResult();
        if (commands == null || commands.length == 0) {
            return commandResult;
        }

        Process process = null;
        DataOutputStream os = null;
        BufferedReader successResult = null;
        BufferedReader errorResult = null;
        StringBuilder successMsg;
        StringBuilder errorMsg;

        try {
            process = Runtime.getRuntime().exec(root ? COMMAND_SU : COMMAND_SH);
            os = new DataOutputStream(process.getOutputStream());
            for (String command : commands) {
                if (command != null) {
                    os.write(command.getBytes());
                    os.writeBytes(COMMAND_LINE_END);
                    os.flush();
                }
            }

            os.writeBytes(COMMAND_EXIT);
            os.flush();
            commandResult.result = process.waitFor();

            // 获取错误信息
            successMsg = new StringBuilder();
            errorMsg = new StringBuilder();
            successResult = new BufferedReader(new InputStreamReader(process.getInputStream()));
            errorResult = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            String s;
            while ((s = successResult.readLine()) != null) {
                successMsg.append(s);
            }
            while ((s = errorResult.readLine()) != null) {
                errorMsg.append(s);
            }
            commandResult.successMsg = successMsg.toString();
            commandResult.errorMsg = errorMsg.toString();

            Log.i(TAG, commandResult.result + " | " + commandResult.successMsg
                    + " | " + commandResult.errorMsg);
        } catch (Exception e) {
            String errMsg = e.getMessage();
            if (errMsg != null) {
                Log.e(TAG, errMsg);
            } else {
                e.printStackTrace();
            }
        } finally {
            try {
                if (os != null) {
                    os.close();
                }
                if (successResult != null) {
                    successResult.close();
                }
                if (errorResult != null) {
                    errorResult.close();
                }
            } catch (IOException e) {
                String errMsg = e.getMessage();
                if (errMsg != null) {
                    Log.e(TAG, errMsg);
                } else {
                    e.printStackTrace();
                }
            }

            if (process != null) {
                process.destroy();
            }
        }

        return commandResult;
    }

    /**
     * 输出信息过滤器
     */
    public interface IOutputInfoFilter {
        public void onInfo(String info);
    }

    /**
     * 执行命令-多条
     * @param command 命令
     * @param root 是否 root
     * @param filter 打印监听
     * @return 退出码
     */
    @SuppressWarnings("UnusedReturnValue")
    public static int execCommand(String command, boolean root, IOutputInfoFilter filter) {
        if (TextUtils.isEmpty(command)) {
            return -1;
        }

        int result = 0;
        Process process = null;
        DataOutputStream os = null;
        BufferedReader outputReader = null;
        BufferedReader errorReader = null;
        String s;

        try {
            process = Runtime.getRuntime().exec(root ? COMMAND_SU : COMMAND_SH);
            os = new DataOutputStream(process.getOutputStream());
            if (command != null) {
                os.write(command.getBytes());
                os.writeBytes(COMMAND_LINE_END);
                os.flush();
            }

            os.writeBytes(COMMAND_EXIT);
            os.flush();

            // 读取标准输出流
            outputReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            while((s = outputReader.readLine()) != null) {
                if (filter != null) {
                    filter.onInfo(s);
                }
            }

            // 等待子进程退出
            result = process.waitFor();

            // 获取错误信息
            StringBuilder errorMsg = new StringBuilder();
            errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            while ((s = errorReader.readLine()) != null) {
                errorMsg.append(s);
            }

            Log.i(TAG, "exitCode" + result + " | " + " error: " + errorMsg);
        } catch (Exception e) {
            String errMsg = e.getMessage();
            if (errMsg != null) {
                Log.e(TAG, errMsg);
            } else {
                e.printStackTrace();
            }
        } finally {
            try {
                if (os != null) {
                    os.close();
                }
                if (outputReader != null) {
                    outputReader.close();
                }
                if (errorReader != null) {
                    errorReader.close();
                }
            } catch (IOException e) {
                String errMsg = e.getMessage();
                if (errMsg != null) {
                    Log.e(TAG, errMsg);
                } else {
                    e.printStackTrace();
                }
            }

            if (process != null) {
                process.destroy();
            }
        }

        return result;
    }
}
