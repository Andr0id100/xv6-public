8800 // Shell.
8801 
8802 #include "types.h"
8803 #include "user.h"
8804 #include "fcntl.h"
8805 
8806 // Parsed command representation
8807 #define EXEC  1
8808 #define REDIR 2
8809 #define PIPE  3
8810 #define LIST  4
8811 #define BACK  5
8812 
8813 #define MAXARGS 10
8814 
8815 struct cmd {
8816   int type;
8817 };
8818 
8819 struct execcmd {
8820   int type;
8821   char *argv[MAXARGS];
8822   char *eargv[MAXARGS];
8823 };
8824 
8825 struct redircmd {
8826   int type;
8827   struct cmd *cmd;
8828   char *file;
8829   char *efile;
8830   int mode;
8831   int fd;
8832 };
8833 
8834 struct pipecmd {
8835   int type;
8836   struct cmd *left;
8837   struct cmd *right;
8838 };
8839 
8840 struct listcmd {
8841   int type;
8842   struct cmd *left;
8843   struct cmd *right;
8844 };
8845 
8846 struct backcmd {
8847   int type;
8848   struct cmd *cmd;
8849 };
8850 int fork1(void);  // Fork but panics on failure.
8851 void panic(char*);
8852 struct cmd *parsecmd(char*);
8853 
8854 // Execute cmd.  Never returns.
8855 void
8856 runcmd(struct cmd *cmd)
8857 {
8858   int p[2];
8859   struct backcmd *bcmd;
8860   struct execcmd *ecmd;
8861   struct listcmd *lcmd;
8862   struct pipecmd *pcmd;
8863   struct redircmd *rcmd;
8864 
8865   if(cmd == 0)
8866     exit();
8867 
8868   switch(cmd->type){
8869   default:
8870     panic("runcmd");
8871 
8872   case EXEC:
8873     ecmd = (struct execcmd*)cmd;
8874     if(ecmd->argv[0] == 0)
8875       exit();
8876     exec(ecmd->argv[0], ecmd->argv);
8877     printf(2, "exec %s failed\n", ecmd->argv[0]);
8878     break;
8879 
8880   case REDIR:
8881     rcmd = (struct redircmd*)cmd;
8882     close(rcmd->fd);
8883     if(open(rcmd->file, rcmd->mode) < 0){
8884       printf(2, "open %s failed\n", rcmd->file);
8885       exit();
8886     }
8887     runcmd(rcmd->cmd);
8888     break;
8889 
8890   case LIST:
8891     lcmd = (struct listcmd*)cmd;
8892     if(fork1() == 0)
8893       runcmd(lcmd->left);
8894     wait();
8895     runcmd(lcmd->right);
8896     break;
8897 
8898 
8899 
8900   case PIPE:
8901     pcmd = (struct pipecmd*)cmd;
8902     if(pipe(p) < 0)
8903       panic("pipe");
8904     if(fork1() == 0){
8905       close(1);
8906       dup(p[1]);
8907       close(p[0]);
8908       close(p[1]);
8909       runcmd(pcmd->left);
8910     }
8911     if(fork1() == 0){
8912       close(0);
8913       dup(p[0]);
8914       close(p[0]);
8915       close(p[1]);
8916       runcmd(pcmd->right);
8917     }
8918     close(p[0]);
8919     close(p[1]);
8920     wait();
8921     wait();
8922     break;
8923 
8924   case BACK:
8925     bcmd = (struct backcmd*)cmd;
8926     if(fork1() == 0)
8927       runcmd(bcmd->cmd);
8928     break;
8929   }
8930   exit();
8931 }
8932 
8933 int
8934 getcmd(char *buf, int nbuf)
8935 {
8936   printf(2, "$ ");
8937   memset(buf, 0, nbuf);
8938   gets(buf, nbuf);
8939   if(buf[0] == 0) // EOF
8940     return -1;
8941   return 0;
8942 }
8943 
8944 
8945 
8946 
8947 
8948 
8949 
8950 int
8951 main(void)
8952 {
8953   static char buf[100];
8954   int fd;
8955 
8956   // Ensure that three file descriptors are open.
8957   while((fd = open("console", O_RDWR)) >= 0){
8958     if(fd >= 3){
8959       close(fd);
8960       break;
8961     }
8962   }
8963 
8964   // Read and run input commands.
8965   while(getcmd(buf, sizeof(buf)) >= 0){
8966     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
8967       // Chdir must be called by the parent, not the child.
8968       buf[strlen(buf)-1] = 0;  // chop \n
8969       if(chdir(buf+3) < 0)
8970         printf(2, "cannot cd %s\n", buf+3);
8971       continue;
8972     }
8973     if(fork1() == 0)
8974       runcmd(parsecmd(buf));
8975     wait();
8976   }
8977   exit();
8978 }
8979 
8980 void
8981 panic(char *s)
8982 {
8983   printf(2, "%s\n", s);
8984   exit();
8985 }
8986 
8987 int
8988 fork1(void)
8989 {
8990   int pid;
8991 
8992   pid = fork();
8993   if(pid == -1)
8994     panic("fork");
8995   return pid;
8996 }
8997 
8998 
8999 
9000 // Constructors
9001 
9002 struct cmd*
9003 execcmd(void)
9004 {
9005   struct execcmd *cmd;
9006 
9007   cmd = malloc(sizeof(*cmd));
9008   memset(cmd, 0, sizeof(*cmd));
9009   cmd->type = EXEC;
9010   return (struct cmd*)cmd;
9011 }
9012 
9013 struct cmd*
9014 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
9015 {
9016   struct redircmd *cmd;
9017 
9018   cmd = malloc(sizeof(*cmd));
9019   memset(cmd, 0, sizeof(*cmd));
9020   cmd->type = REDIR;
9021   cmd->cmd = subcmd;
9022   cmd->file = file;
9023   cmd->efile = efile;
9024   cmd->mode = mode;
9025   cmd->fd = fd;
9026   return (struct cmd*)cmd;
9027 }
9028 
9029 struct cmd*
9030 pipecmd(struct cmd *left, struct cmd *right)
9031 {
9032   struct pipecmd *cmd;
9033 
9034   cmd = malloc(sizeof(*cmd));
9035   memset(cmd, 0, sizeof(*cmd));
9036   cmd->type = PIPE;
9037   cmd->left = left;
9038   cmd->right = right;
9039   return (struct cmd*)cmd;
9040 }
9041 
9042 
9043 
9044 
9045 
9046 
9047 
9048 
9049 
9050 struct cmd*
9051 listcmd(struct cmd *left, struct cmd *right)
9052 {
9053   struct listcmd *cmd;
9054 
9055   cmd = malloc(sizeof(*cmd));
9056   memset(cmd, 0, sizeof(*cmd));
9057   cmd->type = LIST;
9058   cmd->left = left;
9059   cmd->right = right;
9060   return (struct cmd*)cmd;
9061 }
9062 
9063 struct cmd*
9064 backcmd(struct cmd *subcmd)
9065 {
9066   struct backcmd *cmd;
9067 
9068   cmd = malloc(sizeof(*cmd));
9069   memset(cmd, 0, sizeof(*cmd));
9070   cmd->type = BACK;
9071   cmd->cmd = subcmd;
9072   return (struct cmd*)cmd;
9073 }
9074 
9075 
9076 
9077 
9078 
9079 
9080 
9081 
9082 
9083 
9084 
9085 
9086 
9087 
9088 
9089 
9090 
9091 
9092 
9093 
9094 
9095 
9096 
9097 
9098 
9099 
9100 // Parsing
9101 
9102 char whitespace[] = " \t\r\n\v";
9103 char symbols[] = "<|>&;()";
9104 
9105 int
9106 gettoken(char **ps, char *es, char **q, char **eq)
9107 {
9108   char *s;
9109   int ret;
9110 
9111   s = *ps;
9112   while(s < es && strchr(whitespace, *s))
9113     s++;
9114   if(q)
9115     *q = s;
9116   ret = *s;
9117   switch(*s){
9118   case 0:
9119     break;
9120   case '|':
9121   case '(':
9122   case ')':
9123   case ';':
9124   case '&':
9125   case '<':
9126     s++;
9127     break;
9128   case '>':
9129     s++;
9130     if(*s == '>'){
9131       ret = '+';
9132       s++;
9133     }
9134     break;
9135   default:
9136     ret = 'a';
9137     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
9138       s++;
9139     break;
9140   }
9141   if(eq)
9142     *eq = s;
9143 
9144   while(s < es && strchr(whitespace, *s))
9145     s++;
9146   *ps = s;
9147   return ret;
9148 }
9149 
9150 int
9151 peek(char **ps, char *es, char *toks)
9152 {
9153   char *s;
9154 
9155   s = *ps;
9156   while(s < es && strchr(whitespace, *s))
9157     s++;
9158   *ps = s;
9159   return *s && strchr(toks, *s);
9160 }
9161 
9162 struct cmd *parseline(char**, char*);
9163 struct cmd *parsepipe(char**, char*);
9164 struct cmd *parseexec(char**, char*);
9165 struct cmd *nulterminate(struct cmd*);
9166 
9167 struct cmd*
9168 parsecmd(char *s)
9169 {
9170   char *es;
9171   struct cmd *cmd;
9172 
9173   es = s + strlen(s);
9174   cmd = parseline(&s, es);
9175   peek(&s, es, "");
9176   if(s != es){
9177     printf(2, "leftovers: %s\n", s);
9178     panic("syntax");
9179   }
9180   nulterminate(cmd);
9181   return cmd;
9182 }
9183 
9184 struct cmd*
9185 parseline(char **ps, char *es)
9186 {
9187   struct cmd *cmd;
9188 
9189   cmd = parsepipe(ps, es);
9190   while(peek(ps, es, "&")){
9191     gettoken(ps, es, 0, 0);
9192     cmd = backcmd(cmd);
9193   }
9194   if(peek(ps, es, ";")){
9195     gettoken(ps, es, 0, 0);
9196     cmd = listcmd(cmd, parseline(ps, es));
9197   }
9198   return cmd;
9199 }
9200 struct cmd*
9201 parsepipe(char **ps, char *es)
9202 {
9203   struct cmd *cmd;
9204 
9205   cmd = parseexec(ps, es);
9206   if(peek(ps, es, "|")){
9207     gettoken(ps, es, 0, 0);
9208     cmd = pipecmd(cmd, parsepipe(ps, es));
9209   }
9210   return cmd;
9211 }
9212 
9213 struct cmd*
9214 parseredirs(struct cmd *cmd, char **ps, char *es)
9215 {
9216   int tok;
9217   char *q, *eq;
9218 
9219   while(peek(ps, es, "<>")){
9220     tok = gettoken(ps, es, 0, 0);
9221     if(gettoken(ps, es, &q, &eq) != 'a')
9222       panic("missing file for redirection");
9223     switch(tok){
9224     case '<':
9225       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
9226       break;
9227     case '>':
9228       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9229       break;
9230     case '+':  // >>
9231       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9232       break;
9233     }
9234   }
9235   return cmd;
9236 }
9237 
9238 
9239 
9240 
9241 
9242 
9243 
9244 
9245 
9246 
9247 
9248 
9249 
9250 struct cmd*
9251 parseblock(char **ps, char *es)
9252 {
9253   struct cmd *cmd;
9254 
9255   if(!peek(ps, es, "("))
9256     panic("parseblock");
9257   gettoken(ps, es, 0, 0);
9258   cmd = parseline(ps, es);
9259   if(!peek(ps, es, ")"))
9260     panic("syntax - missing )");
9261   gettoken(ps, es, 0, 0);
9262   cmd = parseredirs(cmd, ps, es);
9263   return cmd;
9264 }
9265 
9266 struct cmd*
9267 parseexec(char **ps, char *es)
9268 {
9269   char *q, *eq;
9270   int tok, argc;
9271   struct execcmd *cmd;
9272   struct cmd *ret;
9273 
9274   if(peek(ps, es, "("))
9275     return parseblock(ps, es);
9276 
9277   ret = execcmd();
9278   cmd = (struct execcmd*)ret;
9279 
9280   argc = 0;
9281   ret = parseredirs(ret, ps, es);
9282   while(!peek(ps, es, "|)&;")){
9283     if((tok=gettoken(ps, es, &q, &eq)) == 0)
9284       break;
9285     if(tok != 'a')
9286       panic("syntax");
9287     cmd->argv[argc] = q;
9288     cmd->eargv[argc] = eq;
9289     argc++;
9290     if(argc >= MAXARGS)
9291       panic("too many args");
9292     ret = parseredirs(ret, ps, es);
9293   }
9294   cmd->argv[argc] = 0;
9295   cmd->eargv[argc] = 0;
9296   return ret;
9297 }
9298 
9299 
9300 // NUL-terminate all the counted strings.
9301 struct cmd*
9302 nulterminate(struct cmd *cmd)
9303 {
9304   int i;
9305   struct backcmd *bcmd;
9306   struct execcmd *ecmd;
9307   struct listcmd *lcmd;
9308   struct pipecmd *pcmd;
9309   struct redircmd *rcmd;
9310 
9311   if(cmd == 0)
9312     return 0;
9313 
9314   switch(cmd->type){
9315   case EXEC:
9316     ecmd = (struct execcmd*)cmd;
9317     for(i=0; ecmd->argv[i]; i++)
9318       *ecmd->eargv[i] = 0;
9319     break;
9320 
9321   case REDIR:
9322     rcmd = (struct redircmd*)cmd;
9323     nulterminate(rcmd->cmd);
9324     *rcmd->efile = 0;
9325     break;
9326 
9327   case PIPE:
9328     pcmd = (struct pipecmd*)cmd;
9329     nulterminate(pcmd->left);
9330     nulterminate(pcmd->right);
9331     break;
9332 
9333   case LIST:
9334     lcmd = (struct listcmd*)cmd;
9335     nulterminate(lcmd->left);
9336     nulterminate(lcmd->right);
9337     break;
9338 
9339   case BACK:
9340     bcmd = (struct backcmd*)cmd;
9341     nulterminate(bcmd->cmd);
9342     break;
9343   }
9344   return cmd;
9345 }
9346 
9347 
9348 
9349 
