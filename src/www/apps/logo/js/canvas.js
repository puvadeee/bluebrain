
// [imageName] image file name
function loadSprite(imageName)
{
    // create new image object
    var image = new Image();
    // load image
    image.src = imageName;
    // return image object
    return image;
}

// [imageObject] this is image object which is returned loadImage
// [x] screen x coordinate
// [y] screen y coordinate
// [rotation] rotation angle in radians (normal value 0.0)
// [scale] sprite scale (normal value 1.0)
function drawSprite(ctx, imageObject, x, y, rotation, scale)
{
    var w = imageObject.width;
    var h = imageObject.height;
 
    // save state
    ctx.save();
    // set screen position
    ctx.translate(x, y);
    // set rotation
    ctx.rotate(rotation);
    // set scale value
    ctx.scale(scale, scale);
    // draw image to screen drawImage(imageObject, sourceX, sourceY, sourceWidth, sourceHeight,
    // destinationX, destinationY, destinationWidth, destinationHeight)
    ctx.drawImage(imageObject, 0, 0, w, h, -w/2, -h/2, w, h);
    // restore state
    ctx.restore();
}


