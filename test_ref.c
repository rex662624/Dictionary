#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "tst.h"
#include "bench.c"
//---bloom filter
#include "bloom.h"

#define TableSize 5000000	//bloom filter 的大小
#define HashNumber 2	//有多少個 hash function
//---------------
/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };

#define REF INS
#define CPY DEL

#define BENCH_TEST_FILE "bench_ref.txt"

long poolsize = 2000000*WRDMAX;

/* simple trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}

#define IN_FILE "cities.txt"

int main(int argc, char **argv)
{
    char word[WRDMAX] = "";
    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    int rtn = 0, idx = 0, sidx = 0;
    FILE *fp = fopen(IN_FILE, "r");
    double t1, t2;

    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }
    t1 = tvgetf();

//先 create出一個 bloom filter
    bloom_t bloom = bloom_create(TableSize);

//******memorypool*****
    char *pool = (char *) malloc(poolsize * sizeof(char));
    char *Top = pool;
    while ((rtn = fscanf(fp, "%s",Top)) != EOF) {
        char *p = Top;
        /* insert reference to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) {//沒有insert成功

            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fp);
            return 1;
        } else { //有insert 進資料結構，因此也要加入bloom filter
            bloom_add(bloom,Top);
        }
        idx++;
        Top += (strlen(Top) + 1);
    }
    t2 = tvgetf();
    fclose(fp);
    printf("ternary_tree, loaded %d words in %.6f sec\n", idx,t2-t1);
//****************bench for distribution***********
    /*
            if (argc == 2 && strcmp(argv[1], "--bench") == 0) {

                int stat = bench_test(root, BENCH_TEST_FILE, LMAX);
                tst_free(root);
                free(pool);
                return stat;

            }

    */
    //*********************output
    /*       FILE *output;
           output = fopen("ref.txt", "a");
           if(output!=NULL) {
               fprintf(output, "%.6f\n",t2-t1);
               fclose(output);
           } else
               printf("open file error\n");

    */
//*********************



    for (;;) {
        char *p;
        printf(
            "\nCommands:\n"
            " a  add word to the tree\n"
            " f  find word in tree\n"
            " s  search words matching prefix\n"
            " d  delete word from the tree\n"
            " q  quit, freeing all data\n\n"
            "choice: ");

        if(argc>1 && strcmp(argv[1],"--bench")==0)//a for auto
            strcpy(word, argv[2]);
        else
            fgets(word, sizeof word, stdin);

        p = NULL;
        switch (*word) {
        case 'a':
            printf("enter word to add: ");
            if(argc>1 && strcmp(argv[1],"--bench")==0)
                strcpy(Top, argv[3]);

            else if (!fgets(Top, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(Top);

            p = Top;
            t1 = tvgetf();
            /* FIXME: insert reference to each string */
            res = tst_ins_del(&root, &p, INS, REF);
            t2 = tvgetf();
            if (res) {
                idx++;
                Top += (strlen(Top) + 1);
                printf("  %s - inserted in %.10f sec. (%d words in tree)\n",
                       (char *) res, t2 - t1, idx);
            } else
                printf("  %s - already exists in list.\n", (char *) res);

            if(argc>1 && strcmp(argv[1],"--bench")==0)//a for auto
                goto quit;


            break;
        case 'f':
            printf("find word in tree: ");
            if (!fgets(word, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word);
            t1 = tvgetf();

            //用bloom filter去判斷是否在 tree 內
            if (bloom_test(bloom,word)==1) {
                //version1:bloom filter偵測到在裡面就不走下去了
                t2 = tvgetf();
                printf("  Bloomfilter found %s in %.6f sec.\n",word, t2 - t1);
                printf("  Probability of false positives:%lf\n",pow(1 - exp(-(double)HashNumber /(double) ((double)TableSize /(double) idx)), HashNumber));
                // version2:bloom filter偵測到在裡面,就去走tree(防止偵測錯誤)
                t1 = tvgetf();
                res = tst_search(root, word);
                t2 = tvgetf();
                if(res)
                    printf("  ----------\n  Tree found %s in %.6f sec.\n", (char *) res, t2- t1);
                else
                    printf("  ----------\n  %s not found by tree.\n", word);
            } else
                printf("  %s not found by bloom filter.\n", word);
            break;
        case 's':
            printf("find words matching prefix (at least 1 char): ");

            if(argc>1 && strcmp(argv[1],"--bench")==0)
                strcpy(word, argv[3]);
            else if (!fgets(word, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word);
            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, LMAX);
            t2 = tvgetf();
            if (res) {
                printf("  %s - searched prefix in %.6f sec\n\n", word, t2 - t1);
                for (int i = 0; i < sidx; i++)
                    printf("suggest[%d] : %s\n", i, sgl[i]);
            } else
                printf("  %s - not found\n", word);

            if(argc>1 && strcmp(argv[1],"--bench")==0)//a for auto
                goto quit;
            break;
        case 'd':
            printf("enter word to del: ");
            if (!fgets(word, sizeof word, stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word);
            p = word;
            printf("  deleting %s\n", word);
            t1 = tvgetf();
            /* FIXME: remove reference to each string */
            res = tst_ins_del(&root, &p, DEL, REF);
            t2 = tvgetf();
            if (res)
                printf("  delete failed.\n");
            else {
                printf("  deleted %s in %.6f sec\n", word, t2 - t1);
                idx--;
            }
            break;
quit:
        case 'q':
            tst_free(root);
            return 0;
            break;
        default:
            fprintf(stderr, "error: invalid selection.\n");
            break;
        }
    }
    //pool_free(pool);
    bloom_free(bloom);
    return 0;
}
