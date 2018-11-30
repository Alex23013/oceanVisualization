#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <numeric>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
using namespace std;

class WPoint
{
  public:
    GLfloat s, t;       // Coordenadas de texturas
    GLfloat nx, ny, nz; // Coordenadas de la normal
    GLfloat x, y, z;    // posición del vértice

    WPoint(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    void printWPoint()
    {
        cout << " ( " << x << " , " << y << " , " << z << " ) ";
    }
};

class Wave
{
  private:
    float amplitud;
    float frecuencia;
    float direccion;
    float fase;

  public:
    Wave(float amplitud, float direccion, float frecuencia)
    {
        this->amplitud = amplitud;
        this->frecuencia = frecuencia;
        this->direccion = direccion;
        this->fase = 1.0f;
    }
    ~Wave() {}

    void printWave()
    {
        cout << " [ " << this->amplitud << " , " << this->direccion << " , " << this->frecuencia << " ] ";
    }

    float getAmplitud(){return amplitud;}
    float getFrecuencia(){return frecuencia;}
    float getDireccion(){return direccion;}
    float getFase(){return fase;}
};

class Ocean
{
  public:
    Ocean(int nb_pt_x, int nb_pt_z, int dist_x, int dist_z)
    {
        this->nb_pt_x = nb_pt_x;
        this->nb_pt_z = nb_pt_z;
        this->dist_x = dist_x;
        this->dist_z = dist_z;
    }

    ~Ocean() {}

    bool loadWaveInfo(char *filename);
    void genWPoints(); //crea la malla regular(x_i,0,z_i)
    void display();
    void genTexCoords(unsigned int xTotal, unsigned int zTotal);
    void computeNormals(const vector<unsigned int> &indices, const vector<WPoint> &points);
    vector<unsigned int> genIndices(unsigned int xTotal, unsigned int zTotal);
    float computeHeight (float x, float z, float t);
    void updateHeights(float t);

    //datos para la malla regular
    int nb_pt_x, nb_pt_z;
    float dist_x, dist_z;

    int numWaves; //cantidad de olas
    vector<Wave> waves;
    vector<WPoint> puntos;
    vector<unsigned int> indices;
    GLint texture;
};

bool Ocean::loadWaveInfo(char *filename)
{
    ifstream fichero;
    string namefile = "spectrum.txt";
    fichero.open((namefile));
    string frase, token;
    int line = 1;
    int linea_abre = 1;
    if (fichero.fail())
    {
        cout << "Error al abrir el " << namefile << endl;
        return false;
    }
    else
    {
        while (!fichero.eof())
        {
            getline(fichero, frase);
            istringstream iss(frase);
            int i = 0;
            vector<float> info;
            while (getline(iss, token, ' ') && i < 3)
            {
                string tok(token.c_str());
                string::size_type sz;
                info.push_back(stof(tok, &sz));
                i++;
            }
            //for (std::vector<float>::const_iterator i = info.begin(); i != info.end(); ++i)
            //    std::cout << *i << ' ';
            //cout<<endl;
            Wave w(info[0], info[1], info[2]);
            waves.push_back(w);
        }

        fichero.close();
    }
    cout << "Waves leidas" << waves.size() << endl;
    numWaves = waves.size();
    /*for (int i = 0; i < waves.size();i++)
    {
        waves[i].printWave();
        cout<<endl;
    }*/
    return true;
}

void Ocean::genWPoints()
{
    for (int i = 0; i < nb_pt_z; i++)
        for (int j = 0; j < nb_pt_x; j++)
            puntos.push_back(WPoint(j*dist_x*0.1f,0,i*-dist_z*0.1f));
    
    cout<<"puntos generados"<<puntos.size()<<endl;
}

vector<unsigned int> Ocean ::genIndices(unsigned int xTotal, unsigned int zTotal)
{
    /* In a regular mesh:
    * A is for the evens points rows.
    * B is for the odds points rows.
    * last1 and last2 are the last indices of the last triangle.
    */
    vector<unsigned int> indices(3 * (xTotal - 1) * 2 * (zTotal - 1));
    size_t idx = 0;
    for (unsigned int i = 0; i < zTotal - 1; i++)
    {
        unsigned int idxA = i * xTotal;
        unsigned int idxB = (i + 1) * xTotal;
        unsigned int last1 = idxA;
        unsigned int last2 = idxB;
        for (unsigned int j = 0; j < xTotal - 1; j++)
        {
            idxA++;
            idxB++;

            indices[idx] = last1;
            indices[idx + 1] = idxB;
            indices[idx + 2] = last2;

            last2 = idxA;

            indices[idx + 3] = last1;
            indices[idx + 4] = last2;
            indices[idx + 5] = idxB;
            idx += 6;

            last1 = last2;
            last2 = idxB;
        }
    }
    return indices;
}

const GLfloat floorAmbient[4] = {0.5f, 0.5f, 0.5f, 1.0f};
const GLfloat floorDiffuse[4] = {0.7f, 0.7f, 0.7f, 1.0f};
const GLfloat floorSpecular[4] = {0.9f, 0.9f, 0.9f, 1.0f};
const GLfloat floorShininess = 3.0f;

void Ocean::display()
{
    /* cout<<"draw pouints\n";
    glBegin(GL_POINTS);
    glColor3f(1.5f,0.0f,0.0f);
    for(int i=0;i<puntos.size();i++)
    {
        glVertex3f(puntos[i].x,puntos[i].y,puntos[i].z);
    }	
    glEnd();
    */

    //glBindTexture(GL_TEXTURE_2D, texture);
    glMaterialfv(GL_FRONT, GL_AMBIENT, floorAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, floorDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, floorSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, floorShininess);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glInterleavedArrays(GL_T2F_N3F_V3F, sizeof(WPoint), &puntos[0]);
    //cout<<"puntos"<<puntos.size()<<" indices: "<<indices.size()<<endl;
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);

}

void Ocean::genTexCoords(unsigned int xTotal, unsigned int zTotal)
{
    vector<pair<float, float>> texCoords;
    texCoords.reserve(xTotal * zTotal);
    float sStep = 1.0f / float(xTotal - 1);
    float tStep = 1.0f / float(zTotal - 1);

    for (float s = 0.0f; s <= 1.0f; s += sStep)
    {
        for (float t = 0.0f; t <= 1.0f; t += tStep)
        {
            texCoords.emplace_back(make_pair(s, t));
        }
    }
    cout << "tex Coord: " << texCoords.size();
    for (int i = 0; i < texCoords.size(); i++)
    {
        puntos[i].s = texCoords[i].first;
        puntos[i].t = texCoords[i].second;
    }
}

void Ocean::computeNormals(const vector<unsigned int> &indices, const vector<WPoint> &points)
{
    vector<vector<glm::vec3>> trianglesNors(points.size(), vector<glm::vec3>());
    vector<glm::vec3> vertexNors(points.size(), glm::vec3());

    for (size_t i = 0; i < indices.size(); i += 3)
    {

        const glm::vec3 &first = glm::vec3(points[indices[i]].x, points[indices[i]].y, points[indices[i]].z);
        const glm::vec3 &second = glm::vec3(points[indices[i + 1]].x, points[indices[i + 1]].y, points[indices[i + 1]].z);
        const glm::vec3 &third = glm::vec3(points[indices[i + 2]].x, points[indices[i + 2]].y, points[indices[i + 2]].z);
        const glm::vec3 one = second - first;
        const glm::vec3 two = third - first;
        const glm::vec3 norm = glm::normalize(glm::cross(one, two));

        trianglesNors[indices[i]].push_back(norm);
        trianglesNors[indices[i + 1]].push_back(norm);
        trianglesNors[indices[i + 2]].push_back(norm);
    }

    for (size_t i = 0; i < vertexNors.size(); i++)
    {
        vertexNors[i] = accumulate(trianglesNors[i].begin(), trianglesNors[i].end(),
                                   glm::vec3(0));
        vertexNors[i] /= trianglesNors[i].size();
        puntos[i].nx = vertexNors[i].x;
        puntos[i].ny = vertexNors[i].y;
        puntos[i].nz = vertexNors[i].z;
    }
}

//float M_PI = 1.0f;
float Ocean::computeHeight (float x, float z, float t){
    float height = 0.0;
    float k;
    //cout<<"ComputeHeight process with: "<<numWaves<<"waves\n";
    for( int i=0; i< numWaves; i++ )
    {
        k = 4.0 * M_PI * M_PI * waves[i].getFrecuencia() * waves[i].getFrecuencia() / 9.81;
        //cout<<"waves[i].getFrecuencia()  "<<waves[i].getFrecuencia();
        height += waves[i].getAmplitud() * cos(k*(x*cos(waves[i].getDireccion() )
        +z*sin(waves[i].getDireccion() ) )
        -2.0f*M_PI* waves [i].getFrecuencia()*t + waves [i].getFase() );
    }
    return height;
}

void Ocean:: updateHeights(float time){
    //cout<<"t: "<<time<<endl;
    // cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    for (int i = 0; i < nb_pt_z; i++){
        for (int j = 0; j < nb_pt_x; j++)
            {
               puntos[i*nb_pt_x+j].y=computeHeight(j,i,time);
             //   cout<<puntos[i*nb_pt_x+j].y<<" ";
            }
            //cout<<endl;
    }
    // cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
}
