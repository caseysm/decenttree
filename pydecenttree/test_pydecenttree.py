import numpy;
import pydecenttree;

#Todo: Each of these generate different error messages

#pydecenttree.constructTree()
#should complain, no algorithm

#pydecenttree.constructTree('wrong')
#should complain no list of sequences
#(should it also complain, invalid algorithm name?)

#pydecenttree.constructTree('wrong', ['cat','dog'])
#should complain, only two sequences
#(should it also complain, invalid algorithm name?)

#pydecenttree.constructTree('wrong', ['cat','dog', 'rat'], [0,1,1,0], 0)
#Should return an error saying distance matrix is wrong size.

#pydecenttree.constructTree('NJ-R', ['cat','dog', 'rat'], [0,1,1,1,0,1,1,1,0], 0, -2)
#Should say negative precision is nonsensical

#s = pydecenttree.constructTree('NJ-R', ['cat','dog', 'rat'], [0,1,1,1,0,1,1,1,0], -1, 6, 2)
# print (s)
#should work and output (cat:0.5,dog:0.5,rat:0.5).

# s = pydecenttree.constructTree('NJ-R', ['cat','dog', 'rat'], [[0,1,1] ,[1,0,1] ,[2,2]], -1, 6, 2)
# should fail, because third row only has two items

#s = pydecenttree.constructTree('NJ-R', ['cat','dog', 'rat'], [[0,1,1] ,[1,0,1] ,[1,1,0]], -1, 6, 2)
#print (s)
#should work and output (cat:0.5,dog:0.5,rat:0.5).

#print (','.join(pydecenttree.getAlgorithmNames()))
#print ('\n'.join(pydecenttree.getAlgorithmNames(1)))

#dm = numpy.array([[0,2,2], [2,0,2], [2,2,0]], dtype=numpy.double)
#s = pydecenttree.constructTree('NJ-R', ['cat','dog', 'rat'], dm, -1, 6, 0)

taxa = ['LngfishAu', 'LngfishSA', 'LngfishAf', 'Frog', 'Turtle', 'Sphenodon', 'Lizard', 'Crocodile'
       ,'Bird', 'Human', 'Seal', 'Cow', 'Whale', 'Mouse', 'Rat', 'Platypus', 'Opossum']
dm = numpy.array([
        [0.0000000, 0.4394262, 0.4253346, 0.4739334, 0.6469595, 0.8072464, 0.7697307, 0.7867055, 0.7850803, 0.6959920, 0.6336127, 0.6997504, 0.6793033, 0.7120045, 0.7344446, 0.7318557, 0.6820929],
        [0.4394262, 0.0000000, 0.3370420, 0.6028511, 0.7361390, 0.8116547, 0.7978951, 0.7651678, 0.6988087, 0.7598439, 0.7003997, 0.7643007, 0.7378452, 0.7195253, 0.8065221, 0.7439359, 0.6178323],
        [0.4253346, 0.3370420, 0.0000000, 0.5921847, 0.7020407, 0.8573580, 0.9137406, 0.8639154, 0.7878422, 0.8294442, 0.7505365, 0.7882964, 0.7551778, 0.7706974, 0.8561868, 0.7818997, 0.6850401],
        [0.4739334, 0.6028511, 0.5921847, 0.0000000, 0.6268957, 0.8150864, 0.7705955, 0.8145647, 0.7573321, 0.7166360, 0.6626602, 0.6762896, 0.6809187, 0.7005612, 0.7136883, 0.6974592, 0.6449470],
        [0.6469595, 0.7361390, 0.7020407, 0.6268957, 0.0000000, 0.5416702, 0.5841215, 0.5399763, 0.4849490, 0.5861821, 0.5383720, 0.5458270, 0.5612534, 0.5632785, 0.6088667, 0.5946969, 0.5778748],
        [0.8072464, 0.8116547, 0.8573580, 0.8150864, 0.5416702, 0.0000000, 0.6723304, 0.6249144, 0.5897511, 0.6946471, 0.6787380, 0.6565584, 0.6772154, 0.6833316, 0.7476896, 0.7240949, 0.6518428],
        [0.7697307, 0.7978951, 0.9137406, 0.7705955, 0.5841215, 0.6723304, 0.0000000, 0.7160903, 0.6480652, 0.7386573, 0.7042574, 0.7513723, 0.7675040, 0.6849374, 0.7926459, 0.7473559, 0.6709445],
        [0.7867055, 0.7651678, 0.8639154, 0.8145647, 0.5399763, 0.6249144, 0.7160903, 0.0000000, 0.4906657, 0.7432287, 0.7493887, 0.7488493, 0.8057593, 0.8317762, 0.8413625, 0.7521970, 0.7769550],
        [0.7850803, 0.6988087, 0.7878422, 0.7573321, 0.4849490, 0.5897511, 0.6480652, 0.4906657, 0.0000000, 0.6755495, 0.6691967, 0.6860033, 0.6905877, 0.7565543, 0.7504430, 0.6842440, 0.6705485],
        [0.6959920, 0.7598439, 0.8294442, 0.7166360, 0.5861821, 0.6946471, 0.7386573, 0.7432287, 0.6755495, 0.0000000, 0.2750272, 0.2814737, 0.2961226, 0.3590137, 0.3757584, 0.4477864, 0.4220981],
        [0.6336127, 0.7003997, 0.7505365, 0.6626602, 0.5383720, 0.6787380, 0.7042574, 0.7493887, 0.6691967, 0.2750272, 0.0000000, 0.1931946, 0.2118470, 0.2817599, 0.3180795, 0.3787176, 0.3496224],
        [0.6997504, 0.7643007, 0.7882964, 0.6762896, 0.5458270, 0.6565584, 0.7513723, 0.7488493, 0.6860033, 0.2814737, 0.1931946, 0.0000000, 0.1754464, 0.3199376, 0.3320971, 0.4013694, 0.3449257],
        [0.6793033, 0.7378452, 0.7551778, 0.6809187, 0.5612534, 0.6772154, 0.7675040, 0.8057593, 0.6905877, 0.2961226, 0.2118470, 0.1754464, 0.0000000, 0.3251485, 0.3558385, 0.3925798, 0.3595924],
        [0.7120045, 0.7195253, 0.7706974, 0.7005612, 0.5632785, 0.6833316, 0.6849374, 0.8317762, 0.7565543, 0.3590137, 0.2817599, 0.3199376, 0.3251485, 0.0000000, 0.1447365, 0.4099219, 0.3487804],
        [0.7344446, 0.8065221, 0.8561868, 0.7136883, 0.6088667, 0.7476896, 0.7926459, 0.8413625, 0.7504430, 0.3757584, 0.3180795, 0.3320971, 0.3558385, 0.1447365, 0.0000000, 0.4419960, 0.4050277],
        [0.7318557, 0.7439359, 0.7818997, 0.6974592, 0.5946969, 0.7240949, 0.7473559, 0.7521970, 0.6842440, 0.4477864, 0.3787176, 0.4013694, 0.3925798, 0.4099219, 0.4419960, 0.0000000, 0.3202751],
        [0.6820929, 0.6178323, 0.6850401, 0.6449470, 0.5778748, 0.6518428, 0.6709445, 0.7769550, 0.6705485, 0.4220981, 0.3496224, 0.3449257, 0.3595924, 0.3487804, 0.4050277, 0.3202751, 0.0000000]
    ], dtype=numpy.double)
s = pydecenttree.constructTree('NJ-R', taxa, dm, -1, 10, 2)

print (s)