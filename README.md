ocrad.js
========

OCR in Javascript via Emscripten






What consistently amazes me about Optical Character Recognition isn't its astonishing quality or lack thereof. Rather, it's how utterly unpredictable the results can be. Sometimes there'll be some barely legible block of text that comes through absolutely pristine, and some other time there will be a perfectly clean input which outputs complete garbage. Maybe this is a testament to the sheer difficulty of computer vision or the incredible and underappreciated abilities of the human visual cortex.

At one point, I was talking to someone and I distinctly remembered (I know, all the best stories start this way) a sense of surprise when the person indicated that he had heard of Tesseract, the open source OCR engine. I had appraised it as somewhat more obscure than it evidently was. Some time later, I confided about the incident with a friend, and he said something along the lines of "OCR is one of those fields that everyone comes across once".

I guess I've kind of held onto that thought for a while now, and it certainly seems to have at least a grain of truth. Text embedded into the physical world is more or less our primary means we have for communication and expression. Technology is about building tools that augment human capacity and inevitably entails supplanting some human capability. Data input is a huge bottleneck, and while we're kind of sidestepping the problem with things like QR codes by bringing the digital world into the physical. OCR is just one of those fundamental enabling technologies which ought to be as broad in scope as the set of humans who have interacted with a keyboard.

I can't help but feel that the rather large set of people who have interacted with the problem character recognition have surveyed the available tools and reached the same conclusion as your miniature Magic 8 Ball desk ornament: "Try again later". It doesn't take long for one to discover an instance of perfectly crisp and legible type which results in line noise of such entropy that it'd give DUAL_EC_DRBG a run for its money. "No, there really isn't any way for this to be the state of the art." "Well, I guess if it is, then maybe it'll improve in a few years- technology improves quickly, right?" 

You would think that some analogue of Linus's Law would hold true "given enough eyeballs, all bugs are shallow"- especially if you're dealing with literal eyeballs reading letters.





As with any minor stepping stone on the <strikeout>road to hell</strikeout> relentless trajectory of <link>Atwood's Law, I probably don't need to justify the existence of yet another "x, but now in Javascript!", but I might as well try.



