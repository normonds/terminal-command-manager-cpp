#include <stdio.h>
#include <stdlib.h>           // added for exit() function
#include <unistd.h>
//#include <ncurses.h>
#include <form.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <regex>

using namespace std;

extern int COL_GREEN = 1;
extern int COL_YELLOW = 2;
extern int COL_BLUE = 3;
extern int COL_RED = 5;
extern int COL_LIGHT_GREY = 4;

void fail (char *msg) {
    endwin();
    puts(msg);
    exit(EXIT_FAILURE);
}
std::vector<std::string> regexMatches (std::string stri, std::regex expr, bool debug) {
    std::smatch match;
    std::vector<std::string> ret = {};
    int i = 0;
    if (debug) std::cout << "The following matches and submatches were found:" << std::endl;
    while (std::regex_search (stri, match, expr)) {
        i = 0;
        for (auto x:match) {
            if (debug) std::cout << x << " ";
            if (i==0) ret.push_back(x);
            i++;
        }
        stri = match.suffix().str();
        
        if (debug) std::cout << std::endl;
        //outCommand = match.suffix().str();
    }
    return ret;
}
std::string toLower (std::string data) {
    //std::string data = "Abc";
    std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c){ return std::tolower(c); });
    return data;
}
void strReplace (std::string& str, const std::string& oldStr, const std::string& newStr) {
    std::string::size_type pos = 0u;
    while((pos = str.find(oldStr, pos)) != std::string::npos){
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}
template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

int utf8_strlen(std::string str) {
    int c,i,ix,q;
    for (q=0, i=0, ix=str.length(); i < ix; i++, q++) {
        c = (unsigned char) str[i];
        if (c>=0   && c<=127) i+=0;
        else if ((c & 0xE0) == 0xC0) i+=1;
        else if ((c & 0xF0) == 0xE0) i+=2;
        else if ((c & 0xF8) == 0xF0) i+=3;
        //else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return 0;//invalid utf8
    }
    return q;
}
void formatSplitColoring (int y, int x, std::string stri) {
    if (stri.find('^') != std::string::npos) {
        std::vector<std::string> vect = split(stri, '^');
        mvprintw(y, x, vect[0].c_str());
        attron(COLOR_PAIR(COL_LIGHT_GREY));
        mvprintw(y, x+vect[0].length(), "^");
        attroff(COLOR_PAIR(COL_LIGHT_GREY));
        attron(COLOR_PAIR(COL_GREEN));
        mvprintw(y, x+vect[0].length()+1, vect[1].c_str());
        attroff(COLOR_PAIR(COL_GREEN));
    } else {
        attron(COLOR_PAIR(COL_GREEN));
        mvprintw(y, x, stri.c_str());
        attroff(COLOR_PAIR(COL_GREEN));
    }
}
void printBasicLine (int y, int x, int maxW, std::string stri, bool commandColoring = false) {
    if (stri.length()+x < maxW) {
        while (stri.length()+x < maxW) {
                stri += " ";
        }
        if (commandColoring) {
            formatSplitColoring(y, x, stri);
        } else {
            mvprintw(y, x, stri.c_str());
        }
    } else {
        std::string cut;
        cut = stri.substr(0, maxW-6);
        printBasicLine(y, x, maxW, cut + " ... ", commandColoring);
    }
}

// returns printed lines
int printMultiLine (int y, int x, int maxW, std::string stri, int minLines) {
    int linesMade = 0;
    std::string margForMulti = "  ", margActive = "";
    //width -= shiftX;
    if (stri.length()+x < maxW && minLines<2) {
        printBasicLine(y, x, maxW, stri);
        return 1;
    } else {
        std::string cut;
        while (stri.length() > maxW) {
            cut = stri.substr(0, maxW-1);
            stri = stri.substr(maxW-1);
            //mvprintw(1, 0, cut.c_str());
            if (linesMade > 0) margActive = margForMulti; else margActive = "";
            printBasicLine(y+linesMade, x, maxW, margActive+cut);
            //mvprintw(y, x, cut.c_str());
            linesMade++;
        }
        if (linesMade > 0) margActive = margForMulti; else margActive = "";
        printBasicLine(y+linesMade, x, maxW, margActive+stri);
        linesMade++;
        if (minLines>0) {
            while (linesMade<minLines) {
                printBasicLine(y+linesMade, x, maxW, " ");
                linesMade++;
            }
        }
        return linesMade;
    }
    
}
int printMultiInfoLine (int y, int x, int maxW, std::string stri, int minLines) {
    return printMultiLine(y, x, maxW, stri, minLines);
}

int main (int argc, char **argv) {
    setlocale(LC_ALL, "");
    std::string buildDate =  string(__DATE__) + " " + string(__TIME__);
    // std::replace(buildDate.begin(), buildDate.end(), ' ', '_');
    std::cout << ("built on " + buildDate).c_str() << std::endl;
    auto me = getuid();
    auto myprivs = geteuid();
    bool isRoot = false;
    // if (getuid()) printf("%s", "You are not root!\n");
    // else printf("%s", "OK, you are root.\n");
    if (getuid()) {
        std::cout << "Running as not root\n";
    } else {
        isRoot = true;
        COL_GREEN = 5;
        std::cout << "Running as root" << std::endl;
    }
    
    //signal(SIGINT, signal_callback_handler);
    std::vector<std::string> options = {//7, 5, 16, 8};
    //char *options[] = {
        "info linux^printf 'whoami: ';whoami;printf 'uname -a: ';uname -a;lsb_release -a;lscpu | grep 'CPU(s):\\|Model name\\|per socket\\|CPU MHz\\|Vendor ID';awk '$3==\"kB\"{if ($2>1024^2){$2=$2/1024^2;$3=\"GB\";} else if ($2>1024){$2=$2/1024;$3=\"MB\";}} 1' /proc/meminfo | column -t | grep 'MemTotal\\|MemFree\\|SwapTotal\\|SwapFree'",
        "ps linux^ps -afx",
        "top linux^echo \"press V tx2 mx2\";top -c -E g",
        "linux release info^lsb_release -a",

        "gcloud components update",
        "gcloud config list && echo;echo;gcloud app describe",
        "gcloud projects list",

        "ibmcloud update; ibmcloud plugin update",
        "make active nvm node sudoable^n=$(which node); n=${n%/bin/node}; chmod -R 755 $n/bin/*; sudo cp -r $n/{bin,lib,share} /usr/local",

        "kubernetes ibmcloud info^echo --- NODES ---;kubectl get nodes;echo --- PODS ---;kubectl get pods;echo --- SERVICES ---; kubectl get services",

        "kubernetes conn terminal^pods=$(kubectl get pods -o name) && pod=$(eval echo $pods | cut -d'/' -f 2) && echo kubectl exec -it $pod -- top && eval kubectl exec -it $pod -- byobu",

        "l linux filesystem list al files in directory^ls -ahsXp --color ",
        //"la linux filesystem list files detailed^ls -alhsp --color",
        "ls linux filesystem list files permissions^ls -lpah --color | awk '{k=0;for(i=0;i<=8;i++)k+=((substr($1,i+2,1)~/[rwx]/)*2^(8-i));if(k)printf(\"%0o \",k);print}'",
        "linux format volume sudo^mkfs -t ext4 /dev/someVolume",

        "linux filesystem size disks sudo^fdisk -l",
        "linux filesystem size disks^df -h",
        "linux filesystem size disks^lsblk -o NAME,FSTYPE,LABEL,SIZE,UUID,MOUNTPOINT",
        "linux filesystem files in current directory^find -type f | wc -l",
        "linux filesystem current directory size^du -hs <prompt:directory:.>",
        "linux app show largest files^ncdu -x <prompt:directory:/>",
        "listen linux opened ports^netstat -plnt | grep LISTEN --color=always",
        "linux opened ports sudo^lsof -i -P -n | grep LISTEN --color=always",
        "linux system info^lshw -short",
        "linux system info cpu^lscpu",
        "linux current directory^pwd",
        "linux firewall show iptables sudo^iptables -L",
        "linux edit sudo (@reboot /path/to/script.sh)^crontab -e",
        "linux firewall accept all connections for session sudo^iptables -P INPUT ACCEPT && iptables -P OUTPUT ACCEPT && iptables -P FORWARD ACCEPT && iptables -F",
        "linux systemctl service status^systemctl status <prompt:service name:cron>",

        "ip info url request^curl https://ifconfig.co",
        "ipjson info url request^curl https://ifconfig.co/json",
        "port info url request^curl https://ifconfig.co/port/<prompt:port:8080>",

        "nodejs lts install sudo^curl -sL https://deb.nodesource.com/setup_<prompt:LTS version 10,12,14:14>.x | sudo -E bash - && sudo apt-get install nodejs",
        //"linux^grep -inrI <prompt:string to search> <prompt:file or directory> --color=always | more",
        "grep linux search string in files nonbinary recursive caseinsensitive^grep -rnIi --include \\<prompt:filetype:*.*> \"<prompt:search string>\" <prompt:directory:.> --color=always | more",
        "find linux search file^find <prompt:directory:.> -iname \"*<prompt:string in filenamepath:json>*\" -print 2>/dev/null | grep <prompt:string in filenamepath:json> -i --color=always | more",
        "linux append^echo \"<prompt:string>\" >> <prompt:filename:file.txt>",
        "linux replace text file contents^sed -i \"s/<prompt:search string>/<prompt:replace string>/g\" <prompt:filename:file.txt>",
        "linux show first lines of file^head -<prompt:first n lines:30> <prompt:file:file.txt>",
        //'linux filesystem input^ls <prompt:directory> <prompt:other>',

        "mysqldump --user=<prompt:user:root> --password=<prompt:password:1234> <prompt:database> --result-file <prompt:file to save>",
        "linux^zip <prompt:zipped file:file.zip> <prompt:file to zip:file.txt>",

        "git history^git log --graph --decorate --pretty=oneline --abbrev-commit",
        "github publish^git push -u origin master",

        "npm show glob packages^npm list -g --depth 0",
        "npm show outdated glob packages^npm outdated -g --depth=0",

        "package info^apt-cache show <prompt:package name:google-chrome-stable>",

        "youtube 1 fps^ffmpeg -loop 1 -framerate 1 -i <prompt:image file> -i <prompt:mp3 file> -c:v libx264 -preset veryslow -crf 0 -c:a copy -shortest <prompt:output file:output.mp4>",

        "editor open pipe^apt-cache show nano | nano -",
        "editor^echo \"nano: F1 - help;SHIFT+ALT+4 - toogle word wrap; ALT+U - undo; ALT+N - redo;CTRL+^ - start text mark; CTRL+K - cuts selected text;CTRL+U - paste;F6 - search; ALT+W - repeat search;ALT+C - toogle info box\""

    };
    /* Commandline argument currently unused */
    (void) argc;
    (void) argv;
    bool loop = true;
    int i;
    std::string stri = "", command = "";

    if (argc > 1) { 
        // std::cout << "args:" << argv[1] << std::endl;
        std::string arg;
        arg.assign(argv[1]);
        std::string commSearch = arg + " ";
        for (i=0; i<options.size(); i++) {
            // std::cout << i << argv[1] << options[i].find(commSearch) << std::endl;
            if (options[i].find(arg + " ", 0) == 0 || options[i].find(arg + "^", 0) == 0) {
                loop = false;
                command = options[i];
                break;
            }
        }
    }

    if (loop) {
    std::string search = "";
    char *keyTemp = "";
    char *tempStr = "";

    //(void) c;                       // c is currently unused
    int c, width, lineWidth, height;
    FIELD *field[1];
	FORM  *my_form;
	int ch;
    

    initscr();
    cbreak();
    raw();
    noecho();
    keypad(stdscr, TRUE);


    /* Test to see if terminal has colors */
    if (has_colors() == false) {          fail("Colors unavailable\n");    }
    if (start_color() != OK) {              fail("Unable to start colors\n");    }


    init_pair(COL_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COL_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COL_RED, 6, COLOR_BLACK); // blue
    init_pair(4, 8, COLOR_BLACK);  // grey
    init_pair(5, COLOR_RED, COLOR_BLACK);  // grey
    // 8 grey, 9 bright red, 7 white, 6 blue
    //field[0] = new_field(1, 10, 4, 18, 0, 0);
	//field[1] = new_field(1, 10, 6, 18, 0, 0);
	//field[2] = NULL;
    /* Set field options */
	//set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	//field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */
						/* Field is filled up 		*/
	//set_field_back(field[1], A_UNDERLINE); 
	//field_opts_off(field[1], O_AUTOSKIP);
    /* Create the form and post it */
	//my_form = new_form(field);
	//post_form(my_form);

    getmaxyx(stdscr, height, width);
    lineWidth = width-2;
    int headerHeight = 2;
    refresh();
    //WINDOW * menuwin=newwin(10, width, 3, 0);
    //box(menuwin, 0, 0);
    //set_current_field(my_form, field[0]);
    // refresh();
    //wrefresh(menuwin);
    //keypad(menuwin, TRUE);

    int key, indexStart = 0, index = 0, displayi = 0, displaySize = height-headerHeight-6, indexMax = options.size()-1, lineCurr;
    int addLines = 0, n = 0, maxMadeInfoLines = 2, infoLines = 0, maxi = 0;
    std::vector<std::string> searchedOptions = {}, searchVect = {};
    bool keyPressed = false, firstRun = true, allmatch = false;

    refresh();
    while (loop/*key = wgetch(menuwin) != KEY_F(1)*/) {
        keyPressed = false;

        if (keyPressed || firstRun) {
            //firstRun = false;
            lineCurr = 0;
            searchedOptions = {};

            if (search.length() > 0) {
                searchVect = split(search, ' ');
                std::string out = "";
                for (n=0; n<searchVect.size(); n++) {
                    out += searchVect[n] + " ";
                }
                attron(COLOR_PAIR(COL_GREEN));
                printBasicLine(1,0,lineWidth, out);
                attroff(COLOR_PAIR(COL_GREEN));
                searchedOptions = {};

                for (n=0; n<options.size(); n++) {
                    allmatch = true;
                    stri = toLower(options[n]);
                    for (i=0; i<searchVect.size(); i++) {
                        if (stri.find(toLower(searchVect[i])) != std::string::npos) { // match
                        } else {
                            allmatch = false;
                            break;
                        }
                    }
                    if (allmatch) {
                        searchedOptions.push_back(options[n]);
                    }
                }
            } else {
                searchedOptions = options;
                printBasicLine(1, 0, lineWidth, "");
            }

            indexMax = searchedOptions.size()-1;
            // printBasicLine(1, 0, lineWidth, std::to_string())
            if (index > indexMax) index = indexMax;
            if (index < 0) index = 0;
            if (index > displaySize-1) indexStart = index-displaySize+1;
            if (index < indexStart) indexStart = index;

            if (index < displaySize) indexStart = 0;
            maxi = indexStart+displaySize;
            if (maxi-indexStart<10) maxi = 10;
            for (i = indexStart; i < maxi; i++) {
                if (i < searchedOptions.size()) {
                    if (i == index) attron(A_REVERSE);
                    printBasicLine(lineCurr+headerHeight, 0,  lineWidth,  searchedOptions[i], true);
                    
                    //mvwprintw(menuwin, i+1, 1, options[i]);
                    if (i == index) {
                        attroff(A_REVERSE);
                        infoLines = printMultiInfoLine(displaySize+headerHeight + 2, 0, lineWidth, searchedOptions[i], maxMadeInfoLines);
                        if (infoLines > maxMadeInfoLines) maxMadeInfoLines = infoLines;
                    }
                } else {
                    printBasicLine(i+headerHeight, 0,  lineWidth, " ");
                    refresh();
                }
                lineCurr++;
                refresh();
            }
            //printBasicLine(1, 0,  lineWidth, std::to_string(i));
           
        }

        key = getch();
        switch (key) {
           case KEY_F(1):
                loop = false;
                keyPressed = true;
                break;
            case KEY_UP:
                index--;
                keyPressed = true;
                break;
            case KEY_DOWN:
                index++;
                keyPressed = true;
                break;
            case 3: // ctrl+c
                loop = false;
                command = "";
                break;
            default:
                keyPressed = true;
                //if(isascii(key)) {
                if (isprint(key)) {
                    printMultiLine(0, 0, lineWidth, "pressed printable ascii key: " + std::to_string(key) + " with value " + char(key), 0);
                } else {
                    printMultiLine(0, 0, lineWidth, "pressed unprintable ascii key: " + std::to_string(key), 0);
                }
                if (key == 263) { // backspace
                    search = search.substr(0, search.size()-1);
                } else search += char(key);

                // printMultiLine(2, 0, lineWidth, search, 0);
                // move(2, utf8_strlen(search));
                refresh();
                break;
        }

        //refresh();
        if (key==10) {
            command = searchedOptions[index];
            break; // enter
        }
    } // end while

    refresh();
    /* Wait for user to press enter to exit */
    //getch();
    endwin();

    } // end if loop

    if (command == "") {
        std::cout << "no command executed" << std::endl;
        return 0;
    }
    // std::cout << "start executing" << std::endl;
    std::string outCommand = "";
    if (command.find('^') != std::string::npos) {
        std::vector<std::string> commSplit = split(command, '^');
        
        for (i=1; i<commSplit.size(); i++) {
            outCommand += "^"+commSplit[i];
        }
        outCommand = outCommand.substr(1);
    } else outCommand = command;

    std::cout << "prepare comm:" << " \u001b[33m" + outCommand + "\u001b[0m\n";

    //std::string s ("this subject has a submarine as a subsequence");
    std::regex expr ("(<prompt:.*?>)");
    std::vector<std::string> matches = regexMatches(outCommand, expr, false);
    std::string promptStr;
    std::vector<std::string> promptArr;
    std::string defVal;
    std::string inpt;
    matches.erase( unique( matches.begin(), matches.end() ), matches.end() );
    // std::cout << matches.size();
    for (i=0;i<matches.size();i++) {

        promptStr = matches[i].substr(1, matches[i].size()-2);
        promptArr = split(promptStr, ':');
        
        std::cout << promptArr[1];
        if (promptArr.size() > 2)  std::cout << std::string(" ("+promptArr[2]+")");
        std::cout << ": ";
        std::getline(std::cin, inpt);
        if (inpt == "" && promptArr.size() > 2) inpt = promptArr[2];
        //matches[i] = inpt;
        strReplace(outCommand,matches[i], inpt);
    }
    std::cout << "executing: \u001b[31m" + outCommand + "\u001b[0m\n";
    std::cout << "\n";
    //outCommand = "echo \"peace\"";
    
    if (outCommand.length() > 0) std::system(outCommand.c_str());

    return 0;
}