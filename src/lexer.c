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

fp = fopen("../tests/input.txt", "r") ;
    
if (fp == NULL)
    
{
        
printf("Error: cannot open input.txt\n") ;
      
    return 1;
}

printf("Scanning tokens...\n\n") ;

 while ( (c = getc(fp)) != EOF )
    
    
{
        
              if ( isalpha(c) )
        
{
            char word[64] ;
            int i = 0 ;
            int start_col = col ;

                while ( isalpha(c) || isdigit(c) || c == '_' )
            
        {
                word[i] = c ;
                i++ ;
                col++ ;
                c = getc(fp) ;
     
        } 
    
            word[i] = '\0' ;
    
            ungetc(c, fp) ;

    if( is_keyword(word) )
                        
                      printf ("Line %d, Col %d: KEYWORD  -> %s\n", line, start_col, word) ;
            
        else
            
                 printf ("Line %d, Col %d: ID    -> %s\n", line, start_col, word) ;
        
    }
       
           
    else if (isdigit(c))
       
    {
           
        char numstr[64] ;
            int i = 0  ;
            int start_col = col  ;

            while (isdigit(c))
                
            { 
                
                numstr[i] = c
                    ;

                i++ ;
                
                col++;
    
                c = getc(fp);
    }
            
            numstr[i] = '\0';
  
        ungetc (c, fp) ;

            
        printf ("Line %d, Col %d: INT_LIT  -> %s\n", line, start_col, numstr) ;
        
  
    }
        
                    else if (c == '"')
        
                    
    
    
    {
            
            char strval[64] ;
            
            int i = 0 ;
            
            int start_col = col ;

            c = getc(fp) ;
            
            col++ ;

            
            while (c != '"' && c != EOF)
            
            {
                
                strval[i] = c ;
                i++ ;
                col++ ;
                c = getc(fp) ;
            }
            
            strval[i] = '\0';
        
            col++ ;

    
        printf ("Line %d, Col %d: STRING   -> \"%s\"\n", line, start_col, strval) ;
        
        }
                        
               else if (c == '\n')
      
    {
     
        line++ ;
        col = 1 ;
        
    }
      
else if  ( c == ' ' || c == '\t' ) 
         
    {
       
        col++ ;
        
    }

        
            else
        
    {
            int start_col = col;

            
            if  ( c == '=' )
            
            {

                
                char next = getc(fp);
                
              
                   if  (next == '=') 
                
                {
                    
              printf ("Line %d, Col %d: OP       -> ==\n", line, start_col);
                 
                    col += 2;
                
                
                }
                
                
                   else
                
                
                {
                    ungetc(next, fp);
                
                    
                    printf("Line %d, Col %d: OP       -> =\n", line, start_col);
                 
                    col++;
               
                }
                
             
            }
                
else if  (c == '<' )
            
            {
                
                char next = getc(fp);
                
    if (next == '=' )
                
                {
                    
                    printf ("Line %d, Col %d: OP       -> <=\n", line, start_col) ;
                 
                    col += 2;
                 
                }
                    
                
                   else
                
                {
                      ungetc(next, fp);


                    
                    printf ("Line %d, Col %d: OP       -> <\n", line, start_col);
                 
                     col++;
  
                }
                
            }
                
               else if (c == '>')
             
            {
                 
                  char next = getc(fp);
                
                if  (next == '=')
                
                {
                  
                    printf("Line %d, Col %d: OP       -> >=\n", line, start_col);
                    
                    col += 2;
                
                }
                    
                    else
                
                {
                
                    ungetc(next, fp);
                    
                    printf("Line %d, Col %d: OP       -> >\n", line, start_col);
                    
                    col++;
           
                
                }

                
            }
            
            else if (c == '!')
            
            {
                
                char next = getc(fp) ;
                
                   if (next == '=')
        
                { 
                    
                    printf("Line %d, Col %d: OP       -> !=\n", line, start_col);
                 
                    col += 2 ;
                    
                }
                    
                      else
                
                {

                    
                    ungetc(next, fp);
       
                    printf("Line %d, Col %d: OP       -> !\n", line, start_col);
                    
                    col++;
              
                }
            
            }
                
              else if (c == '&')
            
            {
                
                
                char next = getc(fp) ;
                
                 if (next == '&')

                    
                {
                    printf("Line %d, Col %d: OP       -> &&\n", line, start_col);
                  
                    col += 2;
                
                }
                
                else
                
                {

                    
                    
                    ungetc(next, fp);


                    
                    col++;
            
            }
            
            }
            
            else if (c == '|')
            
            
            {
                
               
                char next = getc(fp);
            
                
                if (next == '|')
                    
            {
                    
                    printf("Line %d, Col %d: OP       -> ||\n", line, start_col);

                    
                    col += 2;
                
                }
                
                else
                
                {
                    
                    ungetc(next, fp);
                
                    col++;
            
                }
            
            }
            
            else if 
                
                ( c == '+' || c == '-' || c == '*' || c == '/' ||
                
                c == '(' || c == ')' || c == '{' || c == '}' ||
            
                c == ';' || c == ',' || c == '.' || c == ':' )
            
            {
                
                
                printf("Line %d, Col %d: OP   -> %c\n", line, start_col, c) ;
            
                col++;
            
            }
            
            else
            
            {
              
                col++;
        
            }
        
        }
    
        
    
    
}

 
    fclose(fp) ;
    
    return 0 ; 
    
}


