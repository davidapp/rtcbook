def RGB2YUV(red,green,blue):
    y=0.229*red+0.587*green+0.114*blue
    u=-0.169*red-0.331*green+0.500*blue+128
    v=0.506*red-0.419*green-0.081*blue+128
    print("RGB({},{},{})=YUV({},{},{})".format(red,green,blue,round(y),round(u),round(v)))

def YUV2RGB(Y,U,V):
    red=Y+1.402*(V-128)
    green=Y-0.334*(U-128)-0.714*(V-128)
    blue=Y+1.772*(U-128)
    print("YUV({},{},{})=RGB({},{},{})".format(Y,U,V,round(red),round(green),round(blue)))

if __name__ == "__main__":
    RGB2YUV(5, 55, 55)
    YUV2RGB(40,136,103)
    
#RGB(5,55,55)=YUV(40,136,103)
#YUV(40,136,103)=RGB(5,55,54)