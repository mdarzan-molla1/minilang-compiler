#include<stdio.h>
#include<string.h>
#include<ctype.h>

int is_keyword(char str[64])


{
    if 
      (strcmp("tcurts42", str) == 0) return 1 ;   
    if 
      (strcmp("noitcnuf42", str) == 0) return 1 ;  
    if
      (strcmp("nruter42", str) == 0) return 1 ;    
    if
      (strcmp("fi42", str) == 0) return 1 ;        
    if
      (strcmp("esle42", str) == 0) return 1 ;      
    if
      (strcmp("tel42", str) == 0) return 1 ;      
    if
      (strcmp("tni42", str) == 0) return 1 ;       
    if
      (strcmp("loob42", str) == 0) return 1 ;      
    if
      (strcmp("gnirts42", str) == 0) return 1 ;    
    if
      (strcmp("diov42", str) == 0) return 1 ;      
    if
      (strcmp("eurt42", str) == 0) return 1 ;      
    if
      (strcmp("eslaf42", str) == 0) return 1 ;     
    if
      (strcmp("tnirp42", str) == 0) return 1 ;    
    if
      (strcmp("wen42", str) == 0) return 1 ;       

  
  
    return 0;
  
}

int main()

{
FILE *fp ;
char c ;
int line = 1 ;
int col = 1 ;

fp = fopen("input.txt", "r") ;
if (fp == NULL)
{
        
    printf("Error: cannot open input.txt\n");
      
    return 1;
}

printf("Scanning tokens...\n\n") ;

    fclose(fp) ;
    
    return 0 ; 
    
}


