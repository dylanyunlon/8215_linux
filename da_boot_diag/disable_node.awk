# 用法: awk -v PAT='ac83xx-nandflash' -f disable_node.awk in.dts > out.dts
# 把 compatible 匹配 PAT 的节点设为 status = "disabled"(已有 status 则替换, 否则插在节点开头); 没匹配到任何节点时退出码 3。
function depth_of(s,   o,c){ o=gsub(/\{/,"{",s); c=gsub(/\}/,"}",s); return o-c }
{ lines[NR]=$0 }
END {
  d=0
  for (i=1;i<=NR;i++){ dep[i]=d; d+=depth_of(lines[i]) }        # dep[i] = depth before line i
  # find node bodies: an opening line "name {" at depth k starts node with body depth k+1
  for (i=1;i<=NR;i++){
    if (lines[i] ~ /\{[[:space:]]*$/ && lines[i] !~ /^[[:space:]]*\/\*/) {
      k=dep[i]+1; st=0; hit=0
      for (j=i+1;j<=NR;j++){
        if (dep[j]==k && lines[j] ~ /^[[:space:]]*\};/) { close_[i]=j; break }
        if (dep[j]==k && lines[j] ~ /^[[:space:]]*compatible[[:space:]]*=/ && lines[j] ~ PAT) hit=1
        if (dep[j]==k && lines[j] ~ /^[[:space:]]*status[[:space:]]*=/) st=j
      }
      if (hit) { n++; if (st) repl[st]=1; else after[i]=1; name[n]=lines[i] }
    }
  }
  for (i=1;i<=NR;i++){
    if (i in repl) { m=lines[i]; sub(/status.*/,"status = \"disabled\";",m); print m } else print lines[i]
    if (i in after) { m=lines[i]; sub(/[^[:space:]].*/,"",m); print m "\tstatus = \"disabled\";" }
  }
  for (x=1;x<=n;x++) printf "disabled node: %s\n", name[x] > "/dev/stderr"
  if (n==0) { print "NO_NODE_MATCHED" > "/dev/stderr"; exit 3 }
}
