#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace hesa_mis_core {
using namespace std;

static int *MISet = nullptr;

#define numNb0 1
#define numNb1 2
#define numNb2 3

//int max_vtx;
//int max_col;
static int **adjactMatrix, *adjactMLen, **adjactMIndex;
static int *nbAdjactLen;
static int *unAdjactLen;
static int **beAdjactMatrix, *beAdjactLen, **beAdjactIndex;
static int *vertex, *vertexIndex, vertexLen;
static int *uVertex, *uVertexIndex, uVertexLen;
static int *vertexFlag, *bestVertexFlag;
static int f, f_best, globe_f;
static int *best_vertex;
static int *tabuListS;

static int *nb0, *nb0Index;
static int nb0Len;
static int *nb1, *nb1Index;
static int nb1Len;
static int *nb2, *nb2Index;
static int nb2Len;

static int *vtxIS;

static double dens;

static int tenuePara;

static void constructSol();
static int multi_tabu_search(int, int);
static void updateAdjVertex(int, int, bool, bool);
static void updatesubAdjVertex(int);
static void updateInfo(int, int, bool, bool);
static void setTabuTenue(int, int, bool, bool);
static void clearAll(int, int *);

static void initialFunc(int spSize)
{
      int x, y;

      adjactMatrix = new int *[spSize];
      adjactMLen = new int [spSize];
      adjactMIndex = new int *[spSize];      
      nbAdjactLen = new int [spSize];
      unAdjactLen = new int [spSize];
      beAdjactMatrix = new int *[spSize];
      beAdjactLen = new int [spSize];
      beAdjactIndex = new int *[spSize];
      vertex = new int [spSize];
      vertexIndex = new int [spSize];
      vertexFlag = new int [spSize];
      bestVertexFlag = new int [spSize];
      uVertex = new int [spSize];
      uVertexIndex = new int [spSize];
      tabuListS = new int [spSize];
      best_vertex = new int [spSize];
      vtxIS = new int [spSize];
      
      nb0 = new int [spSize];
      nb0Index = new int [spSize];
      nb1 = new int [spSize];
      nb1Index = new int [spSize];
      nb2 = new int [spSize];
      nb2Index = new int [spSize];                 
      
      for(x=0; x<spSize; x++)
      {
              adjactMatrix[x] = new int [spSize];
              adjactMIndex[x] = new int [spSize]; 
              beAdjactMatrix[x] = new int [spSize];  
              beAdjactIndex[x] = new int [spSize];                                             
      }    
      
      for(x=0; x<spSize; x++)
      {
              adjactMLen[x] = 0;
              nbAdjactLen[x] = 0;
              beAdjactLen[x] = 0;
              vertex[x] = -1;
              vertexIndex[x] = x;
              vertexFlag[x] = -1;
              bestVertexFlag[x] = -1;
              tabuListS[x] = 0;
              nb0[x] = x;
              nb0Index[x] = x;                       
              
              for(y=0; y<spSize; y++)
              {
                      adjactMatrix[x][y] = 0; 
                      adjactMIndex[x][y] = 0;
                      beAdjactMatrix[x][y] = 0; 
                      beAdjactIndex[x][y] = 0;                                   
              } 
      } 
     
     globe_f = 0;
     
     nb1Len = 0;
     nb2Len = 0;
     vertexLen = 0;
     uVertexLen = 0;
     
     tenuePara = 15;
                        
} 

static void constructSol()
{
     int selV;
     while(1)
     {             
             if(nb0Len > 0)
             {
                       selV = nb0[rand() % nb0Len];
                       updateInfo(selV, 0, false, false);          
             }
             else
                       break;        
     }     
}

static bool expandSetFunc(int Iter)
{
       bool perturb;
       int bestV;
       if(nb0Len != 0)
       {
                 perturb = false;
                 bestV = nb0[rand() % nb0Len];
                 f++;
                 updateInfo(bestV, Iter, perturb, false); 
                 return true;                                  
       }
       else
                 return false;     
}

static bool IntensificationStepSpecial(int Iter)
{
       int iterV;
       int best_delt;
       int selV, beV, bestV; 
       bool perturb;
       bool rtnFlag, expandFlag;
       
       best_delt = -1;
       rtnFlag = false;
       
       expandFlag = expandSetFunc(Iter);
       if(expandFlag == true)
               return true;
       else
       {
               for(iterV=0; iterV<nb1Len; iterV++)
               {
                       selV = nb1[iterV];
                       beV = beAdjactMatrix[selV][0];
                           
                       if(nbAdjactLen[beV] > 1)
                       {
                               if(tabuListS[selV] <= Iter)
                               {
                                        if(nbAdjactLen[beV] > best_delt) 
                                        {
                                               best_delt = nbAdjactLen[beV];
                                               bestV = selV; 
                                               rtnFlag = true;                                                     
                                        } 
                                        else if(nbAdjactLen[beV] == best_delt)
                                        {
                                                 if(unAdjactLen[selV] > unAdjactLen[bestV])
                                                 {
                                                        bestV = selV;             
                                                 }
                                                 else if(unAdjactLen[selV] == unAdjactLen[bestV])
                                                 {
                                                         if(rand() % 2)
                                                         {
                                                              bestV = selV;
                                                         }      
                                                 }                               
                                        }       
                               }
                       }                                                            
               }
               if(rtnFlag == true)
               {
                       perturb = false;
                       updateInfo(bestV, Iter, perturb, true);
                       return true;
               }
               else 
                       return false;
       }     
}

static bool IntensificationStep(int Iter)
{
       int iterV;
       int best_delt;
       int selV, beV, bestV; 
       bool perturb;
       bool rtnFlag, expandFlag;
       
       best_delt = -1;
       rtnFlag = false;

       expandFlag = expandSetFunc(Iter);
       if(expandFlag == true)
               return true;
       else
       {       
               for(iterV=0; iterV<nb1Len; iterV++)
               {
                       selV = nb1[iterV];
                       beV = beAdjactMatrix[selV][0];
                        
                       if(tabuListS[selV] <= Iter)
                       {
                                if(nbAdjactLen[beV] > best_delt) 
                                {
                                       best_delt = nbAdjactLen[beV];
                                       bestV = selV;  
                                       rtnFlag = true;                                                    
                                } 
                                else if(nbAdjactLen[beV] == best_delt)
                                {
                                         if(unAdjactLen[selV] > unAdjactLen[bestV])
                                         {
                                                bestV = selV;              
                                         }
                                         else if(unAdjactLen[selV] == unAdjactLen[bestV])
                                         {
                                                 if(rand() % 2)
                                                 {
                                                      bestV = selV;
                                                 }      
                                         }                               
                                }     
                       }
               }                                                                          
        
        
               if(rtnFlag == true)
               {
                      perturb = false;
                      updateInfo(bestV, Iter, perturb, false);
                      return true;
               } 
               else
                      return false;
        }        
}

static void DiversificationStepSpecial(int Iter)
{
       int bestV, selV, best_delt;
       int x;
       bool perturb;       
       bool selFlag;
       
       perturb = true;
       best_delt = -1;
       selFlag = false;
                                                 
       for(x=0; x<uVertexLen; x++)
       {
               selV = uVertex[x];
               if(unAdjactLen[selV] > best_delt && tabuListS[selV] <= Iter)
               {
                          bestV = selV;
                          best_delt = unAdjactLen[selV];  
                          selFlag = true;        
               }
               else if(unAdjactLen[selV] == best_delt && tabuListS[selV] <= Iter)
               {
                         if(rand() % 2)
                              bestV = selV;
               }
       }
                                                  
       if(selFlag == true)
       {
               f += 1 - beAdjactLen[bestV];                              
               updateInfo(bestV, Iter, perturb, true);
       }        
}

static void DiversificationStep(int Iter)
{
       int bestV, selV, best_delt;
       int x;
       bool perturb;       
       bool selFlag;       
       
       perturb = true;
       selFlag = false;
       
       if(rand() % 2)
       {
                   best_delt = -1;
                   for(x=0; x<nb2Len; x++)
                   {
                           selV = nb2[x];
                           if(unAdjactLen[selV] > best_delt && tabuListS[selV] <= Iter)
                           {
                                      bestV = selV;
                                      best_delt = unAdjactLen[selV];
                                      selFlag = true;          
                           }
                           else if(unAdjactLen[selV] == best_delt && tabuListS[selV] <= Iter)
                           {
                                     if(rand() % 2)
                                          bestV = selV;
                           }
                   } 
                   
                   if(selFlag == true)
                   { 
                               f--;                                                                               
                               updateInfo(bestV, Iter, perturb, false);
                   } 
       }
       else
       {
                                                              
                   if(uVertexLen != 0)
                   {
                             bestV = uVertex[rand() % uVertexLen];
                             selFlag = true; 
                   }
                   if(selFlag == true)
                   {
                               f += 1 - beAdjactLen[bestV];                              
                               updateInfo(bestV, Iter, perturb, false);
                   }                          
       }         
}

static int multi_tabu_search(int maxIters, int objSz)
{       
       int Iter, x;
       bool rtnFlag;
       
       rtnFlag = false;
       
       f = vertexLen;
       f_best = f;
       for(x=0; x<f; x++)
       {
           best_vertex[x] = vertex[x];
       }
       
       if(f_best == objSz)
       {
                for(x=0; x<f_best; x++)
                        bestVertexFlag[best_vertex[x]] = 1;
                   
                 return f_best;
       }
       
       for(Iter=1; Iter < maxIters ; Iter++)
       {  
                 if(nb1Len > nb2Len + uVertexLen)
                 {
                           rtnFlag = IntensificationStepSpecial(Iter);
                           if(rtnFlag == false)
                                     DiversificationStepSpecial(Iter);
                 }
                 else
                 {
                           rtnFlag = IntensificationStep(Iter);
                           if(rtnFlag == false)
                                     DiversificationStep(Iter);
                 } 
                 
                 if(f >= f_best)
                 {
                         if(f > f_best)
                         {
                             f_best = f;
                             for(x=0; x<f_best; x++)
                             {
                                    best_vertex[x] = vertex[x];
                             }
                         }

                         if(f_best == objSz)
                         {
                                 for(x=0; x<f_best; x++)
                                 {
                                        bestVertexFlag[best_vertex[x]] = 1;
                                 }                                 
                                 return f_best;
                         }
                 }                
       }
       
       for(x=0; x<f_best; x++)
       {
            bestVertexFlag[best_vertex[x]] = 1;
       }
       return f_best;              
}

static void setTabuTenue(int v, int Iter, bool perturbFlag, bool tabuLongFlag)
{
       if(perturbFlag == false)
       {
                  if(tabuLongFlag == false)
                              tabuListS[v] = Iter + tenuePara + rand() % (nb1Len+1);
                  else
                              tabuListS[v] = Iter + nb1Len; 
       }
       else 
       {
                  tabuListS[v] = Iter + 7; 
       }     
}

static void updatesubAdjVertex(int adjV)
{
       int y;
       int subAdjV;
       int beIndex, beV, nbId, nbValue, index, tmpV;
       
       for(y=0; y<adjactMLen[adjV]; y++)
       {
                subAdjV = adjactMatrix[adjV][y];
                
                if(vertexFlag[subAdjV] != 1)
                {
                        unAdjactLen[adjV]++;
                        unAdjactLen[subAdjV]++;                            
                        
                        beIndex = beAdjactIndex[subAdjV][adjV]; 
                        beAdjactLen[subAdjV]--;

                        beV = beAdjactMatrix[subAdjV][beAdjactLen[subAdjV]];      
                        beAdjactMatrix[subAdjV][beIndex] = beV;
                        beAdjactIndex[subAdjV][beV] = beIndex; 
                        if(beAdjactLen[subAdjV] == 0)
                        {
                                        nb0[nb0Len] = subAdjV;
                                        nb0Index[subAdjV] = nb0Len;
                                        nb0Len++; 
                                        
                                        nb1Len--;
                                        nbId = nb1Index[subAdjV];
                                        nbValue = nb1[nb1Len]; 
                                        nb1[nbId] = nbValue;
                                        nb1Index[nbValue] = nbId;                                                                                                                                                        
                        }
                        else if(beAdjactLen[subAdjV] == numNb0)
                        {
                                        nb1[nb1Len] = subAdjV;
                                        nb1Index[subAdjV] = nb1Len;
                                        nb1Len++; 
                                        
                                        beV = beAdjactMatrix[subAdjV][0];
                                        nbAdjactLen[beV]++;                                            
                                        
                                        nb2Len--;
                                        nbId = nb2Index[subAdjV];
                                        nbValue = nb2[nb2Len]; 
                                        nb2[nbId] = nbValue;
                                        nb2Index[nbValue] = nbId;                                                                                                                                                     
                        } 
                        else if(beAdjactLen[subAdjV] == numNb1)
                        {
                                        nb2[nb2Len] = subAdjV;
                                        nb2Index[subAdjV] = nb2Len;
                                        nb2Len++;
                                        
                                        index = uVertexIndex[subAdjV];
                                        uVertexLen--;
                                        tmpV = uVertex[uVertexLen];
                                        uVertex[index] = tmpV;
                                        uVertexIndex[tmpV] = index;                                                    
                        } 
                                                      
                }                                                     
        }     
}


static void updateAdjVertex(int bestV, int Iter, bool perturb, bool tabuLong)
{
       int x;
       int adjV;
       int index, tmpV, nbId, nbValue, beV; 
       bool changeState = false;
       
       for(x=0; x<adjactMLen[bestV]; x++)
       {
                adjV = adjactMatrix[bestV][x];

                if(vertexFlag[adjV] != 1)
                {
                        unAdjactLen[adjV]--;
                        changeState = false;
                }
                else if(vertexFlag[adjV] == 1)
                {
                           beAdjactLen[adjV] = 0;
                           
                           index = vertexIndex[adjV];
                           vertexLen--;
                           vertexFlag[adjV] = 0;
                           tmpV = vertex[vertexLen];
                           vertex[index] = tmpV; 
                           vertexIndex[tmpV] = index;
                           changeState = true;                                                   
                           
                           setTabuTenue(adjV, Iter, perturb, tabuLong);
                                      
                           nbAdjactLen[adjV] = 0; 
                           
                           updatesubAdjVertex(adjV);
                }
                     
                beAdjactMatrix[adjV][beAdjactLen[adjV]] = bestV;
                beAdjactIndex[adjV][bestV] = beAdjactLen[adjV];
                beAdjactLen[adjV]++;
                
                if(beAdjactLen[adjV] == numNb0)
                {
                                 if(changeState == false)
                                 {
                                        nb0Len--;
                                        nbId = nb0Index[adjV];
                                        nbValue = nb0[nb0Len]; 
                                        nb0[nbId] = nbValue;
                                        nb0Index[nbValue] = nbId;
                                 }
                                
                                 nb1[nb1Len] = adjV;
                                 nb1Index[adjV] = nb1Len;
                                 nb1Len++;

                                 nbAdjactLen[bestV]++;
                                                                                                                                         
                }            
                else if(beAdjactLen[adjV] == numNb1)
                {
                                nb1Len--;
                                nbId = nb1Index[adjV];
                                nbValue = nb1[nb1Len]; 
                                nb1[nbId] = nbValue;
                                nb1Index[nbValue] = nbId;
                                
                                beV = beAdjactMatrix[adjV][0];
                                nbAdjactLen[beV]--;
                                
                                nb2[nb2Len] = adjV;
                                nb2Index[adjV] = nb2Len;
                                nb2Len++;                                                                  
                }
                else if(beAdjactLen[adjV] == numNb2)
                {
                                nb2Len--;
                                nbId = nb2Index[adjV];
                                nbValue = nb2[nb2Len]; 
                                nb2[nbId] = nbValue;
                                nb2Index[nbValue] = nbId;

                                uVertex[uVertexLen] = adjV;
                                uVertexIndex[adjV] = uVertexLen;
                                uVertexLen++;                                                                  
                }                        
       
       }     
}

static void updateInfo(int bestV, int Iter, bool perturb, bool tabuLong)
{
       int nbId, nbValue;
       
       vertex[vertexLen] = bestV;
       vertexIndex[bestV] = vertexLen;
       vertexLen++;
       vertexFlag[bestV] = 1;
       unAdjactLen[bestV] = 0;
        
       if(beAdjactLen[bestV] == 0)
       {
               nbId = nb0Index[bestV];
               nb0Len--;
               nbValue = nb0[nb0Len]; 
               nb0[nbId] = nbValue;
               nb0Index[nbValue] = nbId;     
       }
       else if(beAdjactLen[bestV] == numNb0)
       {
               nbId = nb1Index[bestV];
               nb1Len--;
               nbValue = nb1[nb1Len]; 
               nb1[nbId] = nbValue;
               nb1Index[nbValue] = nbId;    
       } 
       else if(beAdjactLen[bestV] == numNb1)
       {
               nbId = nb2Index[bestV];
               nb2Len--;
               nbValue = nb2[nb2Len]; 
               nb2[nbId] = nbValue;
               nb2Index[nbValue] = nbId;    
       } 
       else if(beAdjactLen[bestV] >= numNb2)
       {
               nbId = uVertexIndex[bestV];
               uVertexLen--;
               nbValue = uVertex[uVertexLen];
               uVertex[nbId] = nbValue;
               uVertexIndex[nbValue] = nbId;           
       }  
       
       updateAdjVertex(bestV, Iter, perturb, tabuLong);
       
       beAdjactLen[bestV] = 0;                                         
}

static void clearAll(int spSize, int *nb0Source)
{
          int x;
          
          for(x=0; x<spSize; x++)
          {
                  beAdjactLen[x] = 0;
                  nbAdjactLen[x] = 0;
                  vertex[x] = -1;
                  vertexIndex[x] = x;
                  vertexFlag[x] = -1;
                  bestVertexFlag[x] = -1;
                  
                  nb0[x] = nb0Source[x];
                  nb0Index[nb0[x]] = x;
                  unAdjactLen[x] = adjactMLen[x];
               
                  tabuListS[x] = 0;
                  best_vertex[x] = 0;                                                      
          }  
  
          nb0Len = spSize;
          
          nb1Len = 0;
          nb2Len = 0;
          uVertexLen = 0;
          vertexLen = 0;
}

bool verifyIndependentSize(int mSize, int *MIS)
{
      for(int x=0; x<mSize-1; x++)
      {
              for(int y=x+1; y<mSize; y++)
              {
                      if(MIS[x]==MIS[y])
                           return false;
              }
      }
      return true;
}

#undef numNb0
#undef numNb1
#undef numNb2

static void freeSpace(int spSize)
{
      int x;
      
      for(x=0; x<spSize; x++)
      {
          delete []adjactMatrix[x];
          delete []adjactMIndex[x];
          delete []beAdjactMatrix[x];
          delete []beAdjactIndex[x];
      }
      delete  []adjactMatrix;
      delete  []adjactMIndex;
      delete  []adjactMLen;
      
      delete  []beAdjactMatrix;
      delete  []beAdjactIndex;
      delete  []nbAdjactLen;
      delete  []unAdjactLen;
      delete  []beAdjactLen;
      delete  []vertex;
      delete  []vertexIndex;
      delete  []vertexFlag;
      delete  []bestVertexFlag;
      delete  []uVertex;
      delete  []uVertexIndex;
      delete  []tabuListS;
      delete  []best_vertex;
      delete  []vtxIS;
      
      delete  []nb0;
      delete  []nb0Index;
      delete  []nb1;
      delete  []nb1Index;
      delete  []nb2;
      delete  []nb2Index;      
}

int MISFunc(int *inVtx, int inVtxLen, int objSz, int **edgeMatrix, int lanchIters)
{
       int x, y, i;
       int selV, adjV;
       int iters;
       int globe_f = 0;
       int maxSize;
       int Val;
       int len;
       
       initialFunc(inVtxLen);       

       //cout<<"inVtxLen="<<inVtxLen<<endl;

       nb0Len = inVtxLen;
       for(i=0; i<inVtxLen; i++)
       {
             selV = inVtx[i];
             nb0[i] = selV;
             nb0Index[selV] = i;    
       }
       //cout<<"nb0Len="<<nb0Len<<endl;
       for(x=0 ; x<nb0Len; x++ )
       {
             selV = nb0[x];
             adjactMLen[selV] = 0;
             unAdjactLen[selV] = 0;
             for(y=0; y<nb0Len; y++ )
             {
                     adjV = nb0[y];
                     if( edgeMatrix[selV][adjV] == 1 )
                     {
                        adjactMatrix[selV][adjactMLen[selV]] = adjV;
                        adjactMIndex[selV][adjV] = adjactMLen[selV];
                        adjactMLen[selV]++;
    
                        unAdjactLen[selV]++;
                     }
             }
       }                     
       
       globe_f = 0; 
       for(iters = 0; iters < lanchIters; iters++)
       {
           constructSol();
           
           maxSize = multi_tabu_search(10000, objSz);

           //cout<<"iters="<<iters<<", maxSize="<<maxSize<<", globe_f="<<globe_f<<endl;
           if(maxSize > globe_f)
           {
                           globe_f =  maxSize;
                           for(x=0; x<globe_f; x++)
                                  MISet[x] = best_vertex[x];                                                                                                                         
           }
           
           clearAll(inVtxLen, inVtx);
       }

       freeSpace(inVtxLen);     
       return globe_f;  
}



}
