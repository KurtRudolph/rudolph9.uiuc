%cs412 HW1 

%problem 1

price = [11.78, 85.12, 10.47, 298.00, 38.45, 102.14, 123.62, 203.29, 65.00, 225.50, 9.25, 164.32, 102.45, 120.45, 73.15, 625.00, 125.00, 242.64, 441.76, 325.45];
weight = [3.2, 3.4, 4.5, 35.4, 9.1, 5.7, 1.5, 23.8, 8.6, 42.3, 5.9, 12.3, 6.5, 11.8, 12.2, 32.9, 11.6, 48.0, 52.9, 78.2];

priceMean = mean(price)
priceMedian = median(price)
priceStd = std(price)
priceQ1 = prctile(price, 25)
priceQ2 = prctile(price, 75)

weightMean = mean(weight)
weightMedian = median(weight)
weightStd = std(weight)
weightQ1 = prctile(weight, 25)
weightQ2 = prctile(weight, 75)
