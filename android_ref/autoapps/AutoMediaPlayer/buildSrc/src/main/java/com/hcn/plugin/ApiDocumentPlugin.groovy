package com.hcn.plugin

import org.gradle.api.Plugin
import org.gradle.api.Project

import java.text.SimpleDateFormat

/**
 * 把中文 API 接口文档更新到 rootProject/docs 目录
 * <p> 这个插件是为了学习测试使用而生（首次研究插件的例子）
 */
class ApiDocumentPlugin implements Plugin<Project> {

    @Override
    void apply(Project project) {
        // 扩展
        project.extensions.create('document', DocumentExtension)

        project.tasks.register('documentTask') {
            doLast {
                println "documentTask start..."

                def ext = project['document'] as DocumentExtension
                def apiReadme = ext.apiReadmeFile
                def apiDocument = ext.apiDocumentFile

                publicApiDocument(apiReadme, apiDocument)

                println "documentTask finished."
            }
        }
    }

    static def publicApiDocument(File readme, File document) {
        FormatUtils.format(readme)
        def lines = readme.readLines("UTF-8")
        def sb = new StringBuilder()

        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss")
        sb.append("#### [<font color=\"#eeccaa\">Update Time:</font> " + format.format(new Date()) + "]")
        sb.append(FormatUtils.LINE_SEP)
        sb.append(FormatUtils.LINE_SEP)

        readme.eachLine { line ->
            sb.append(line)
            sb.append(FormatUtils.LINE_SEP)
        }
        document.write(sb.toString(), "UTF-8")
    }
}
