#include <mitkImageToItk.h>
#include <itkCastImageFilter.h>

/*!
* \brief method creates a cast image filter and casts the image of ItkInputImageType
* to type ItkOutputImageType. The method needs to be called via the 
* mitkImageToItkMultiplexer 
*
* \code
*  mitk::Image::Pointer inputMitkImage;
*  ... // set inputMitkImage
*  ITKImageType::Pointer resultItkImage = ITKImageType::New();
*  FixedInputDimensionMitkToItkFunctionMultiplexer(resultItkImage, ITKImageType, inputMitkImage, dimension, MakeCastImageFilter);
*
* \endcode
*/
template <  typename  ItkInputImageType,  typename ItkOutputImageType > 
itk::ImageSource < ItkOutputImageType > ::Pointer 
MakeCastImageFilter(  ItkInputImageType* inputImage )
{
  typedef itk::CastImageFilter < ItkInputImageType , 
    ItkOutputImageType > myFilterType;
  myFilterType::Pointer myFilter = myFilterType::New();
  myFilter->SetInput( inputImage );
  return myFilter.GetPointer();
}

#define _calculateItkPipelineFunction(result, mitksource, itkpipelinefunction, type, dimension)    \
  if ( typeId == typeid(type) )                                                                  \
{                                                                                              \
    typedef mitk::ImageToItk<type, dimension> ImageToItkType;                                  \
    typedef itk::Image<type, dimension> ImageType;                                  \
    typedef itk::ImageSource<ImageType> ImageFilterType;                                \
    \
    ImageToItkType::Pointer myImageToItkFilter = ImageToItkType::New();                                \
    myImageToItkFilter ->SetInput(mitksource);                                                  \
    \
    ImageFilterType::Pointer itkpipeline =                                                     \
    itkpipelinefunction(myImageToItkFilter->GetOutput()).GetPointer();                             \
    itkpipeline->Update();                                                                 \
    \
    result->InitializeByItk(itkpipeline->GetOutput());                                              \
    result->SetVolume(itkpipeline->GetOutput()->GetBufferPointer());                           \
}                                                              
//		_calculateItkPipelineFunction(result, mitkimage, itkpipeline<type, dimension>::New());     

#define ItkFunctionMultiplexer(result, mitkSourceImage, itkfunction)                                     \
{                                                                                                  \
  const std::type_info& typeId=*mitkSourceImage->GetPixelType().GetTypeId();                                     \
  switch (mitkSourceImage->GetDimension())                                                             \
{                                                                                                  \
    case 2:                                                                                      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, double, 2) else            \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, float, 2) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, long, 2) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned long, 2) else     \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, int, 2) else               \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned int, 2) else      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, short, 2) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned short, 2) else    \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, char, 2) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned char,  2)         \
      break;                                                                                         \
    case 3:                                                                                          \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, double, 3) else            \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, float, 3) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, long, 3) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned long, 3) else     \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, int, 3) else               \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned int, 3) else      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, short, 3) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned short, 3) else    \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, char, 3) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned char,  3)         \
      break;                                                                                         \
    case 4:                                                                                          \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, double, 4) else            \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, float, 4) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, long, 4) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned long, 4) else     \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, int, 4) else               \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned int, 4) else      \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, short, 4) else             \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned short, 4) else    \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, char, 4) else              \
      _calculateItkPipelineFunction(result, mitkSourceImage, itkfunction, unsigned char,  4)         \
      break;                                                                                         \
    default: break;                                                                                  \
}                                                                                                    \
}
///////////////////////////////////////////////

#define _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitksource, itkpipelinefunction, type, dimension)    \
  if ( typeId == typeid(type) )                                                                  \
{                                                                                              \
    typedef mitk::ImageToItk<type, dimension> ImageToItkType;                                  \
    typedef itk::Image<type, dimension> ImageType;                                  \
    typedef itk::ImageSource<resultItkImageType> ImageFilterType;                                \
    \
    ImageToItkType::Pointer myImageToItkFilter = ImageToItkType::New();                                \
    myImageToItkFilter->SetInput(mitksource);                                                  \
    \
    ImageFilterType::Pointer itkpipeline =                                                     \
    itkpipelinefunction<ImageType,resultItkImageType>(myImageToItkFilter->GetOutput()).GetPointer();                             \
    itkpipeline->Update();                                                                 \
    \
    resultItkImage = itkpipeline->GetOutput();                                              \
}                                                              

//##Documentation
//## @brief multiplexes a mitk image to an itkFilter with inputs of dimensions 2-4 and all data types (the itkFilter must allow to do this!).
#define AllInputDimensionMitkToItkFunctionMultiplexer(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction)                                     \
{                                                                                                  \
  const std::type_info& typeId=*mitkSourceImage->GetPixelType().GetTypeId();                                     \
  switch (mitkSourceImage->GetDimension())                                                             \
{                                                                                                  \
    case 2:                                                                                      \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, double, 2) else            \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, float, 2) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, long, 2) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned long, 2) else     \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, int, 2) else               \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned int, 2) else      \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, short, 2) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned short, 2) else    \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, char, 2) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned char,  2)         \
      break;                                                                                         \
    case 3:                                                                                          \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, double, 3) else            \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, float, 3) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, long, 3) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned long, 3) else     \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, int, 3) else               \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned int, 3) else      \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, short, 3) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned short, 3) else    \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, char, 3) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned char,  3)         \
      break;                                                                                         \
    case 4:                                                                                          \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, double, 4) else            \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, float, 4) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, long, 4) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned long, 4) else     \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, int, 4) else               \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned int, 4) else      \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, short, 4) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned short, 4) else    \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, char, 4) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned char,  4)         \
      break;                                                                                         \
    default: break;                                                                                  \
}                                                                                                    \
}

//##Documentation
//## @brief multiplexes a mitk image to an itkFilter with inputs of fixed dimensions @a inputDimension and all data types (the itkFilter must allow to do this!).
#define FixedInputDimensionMitkToItkFunctionMultiplexer(resultItkImage, resultItkImageType, mitkSourceImage, inputDimension, itkfunction)                                     \
{                                                                                                  \
  const std::type_info& typeId=*mitkSourceImage->GetPixelType().GetTypeId();                                     \
  if (mitkSourceImage->GetDimension() == inputDimension)                                                             \
  {                                                                                                  \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, double, inputDimension) else            \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, float, inputDimension) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, long, inputDimension) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned long, inputDimension) else     \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, int, inputDimension) else               \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned int, inputDimension) else      \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, short, inputDimension) else             \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned short, inputDimension) else    \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, char, inputDimension) else              \
      _calculateMitkToItkPipelineFunction(resultItkImage, resultItkImageType, mitkSourceImage, itkfunction, unsigned char,  inputDimension)         \
  }                                                                                                    \
}
